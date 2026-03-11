#include "client.h"
#include "base64.h"
#include "session.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <termios.h>
#include "../voice/voice_call.hpp"

// ── Globals ───────────────────────────────────────────────────────────────────
static VoiceCall voiceCall;
static int clientFd = -1;
std::unordered_map<std::string, CryptoSession> sessionStore;

// ── UI State ──────────────────────────────────────────────────────────────────
enum class Screen { AUTH, MENU, DM, ROOM };
enum class AuthStep { USERNAME, CONFIRM_SIGNUP, PASSWORD };

struct UIState
{
    Screen   screen         = Screen::AUTH;
    AuthStep authStep       = AuthStep::USERNAME;
    bool     signingUp      = false;
    bool     passwordMode   = false;   // true → readLine must not echo
    std::string pendingUsername;
    std::string username;
    std::string target;
    std::string historyPeer;
} ui;

// ── File transfer state ───────────────────────────────────────────────────────
static std::string pendingFilePath;
static std::string incomingFilename;
static std::string incomingSender;
static size_t      incomingFilesize = 0;
static std::vector<uint8_t> incomingFileData;

// ── Terminal raw mode ─────────────────────────────────────────────────────────
static struct termios origTermios;

static void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &origTermios);
    struct termios raw = origTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
}

// ── Prompt ────────────────────────────────────────────────────────────────────
static std::string prompt()
{
    switch (ui.screen)
    {
    case Screen::AUTH: return "> ";
    case Screen::MENU: return "[" + ui.username + "] > ";
    case Screen::DM:   return "[" + ui.username + " \xe2\x86\x92 " + ui.target + "] > ";
    case Screen::ROOM: return "[" + ui.username + " @ " + ui.target + "] > ";
    }
    return "> ";
}

// ── Output ────────────────────────────────────────────────────────────────────
static std::mutex printMtx;
static std::string inputBuf;

static void printMsg(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(printMtx);
    std::cout << "\r\033[K" << msg << "\n" << prompt() << inputBuf << std::flush;
}

// ── Screens ───────────────────────────────────────────────────────────────────
static void showAuthScreen()
{
    std::cout << "\033[2J\033[H";
    std::cout << "\xe2\x95\x94\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x97\n";
    std::cout << "\xe2\x95\x91        ChatApp           \xe2\x95\x91\n";
    std::cout << "\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x9d\n\n";
    std::cout << "Enter username: " << std::flush;
}

static void showMenu()
{
    std::cout << "\033[2J\033[H";
    std::cout << "Logged in as " << ui.username << ".\n\n";
    std::cout << "  dm     <user>          start a DM\n";
    std::cout << "  join   <room>          join a room\n";
    std::cout << "  create <room>          create a room\n";
    std::cout << "  users                  list users\n";
    std::cout << "  rooms                  list rooms\n";
    std::cout << "  history dm   <user>    DM history\n";
    std::cout << "  history room <room>    room history\n";
    std::cout << "  logout\n\n";
    std::cout << prompt() << std::flush;
}

static void showChatHelp()
{
    printMsg("  /send <filepath>   send a file");
    printMsg("  /call              start a voice call");
    printMsg("  /history           show chat history");
    printMsg("  /accept            accept incoming file or call");
    printMsg("  /reject            reject incoming file or call");
    printMsg("  /endcall           end active call");
    printMsg("  /q                 go back to menu");
}

// ── History decryption ────────────────────────────────────────────────────────
static std::string tryDecryptHistoryLine(const std::string &line)
{
    if (ui.historyPeer.empty())
        return line;

    if (!sessionStore.count(ui.historyPeer) || !sessionStore[ui.historyPeer].ready)
    {
        CryptoSession s;
        if (!s.load(ui.historyPeer))
            return line;
        sessionStore[ui.historyPeer] = s;
    }

    auto &session = sessionStore[ui.historyPeer];

    auto sep = line.rfind(": ");
    if (sep == std::string::npos)
        return line;

    std::string prefix  = line.substr(0, sep + 2);
    std::string content = line.substr(sep + 2);
    if (content.empty())
        return line;

    for (char c : content)
        if (!isalnum(c) && c != '+' && c != '/' && c != '=')
            return line;

    try
    {
        std::string plain = session.decryptMsg(content);
        for (unsigned char c : plain)
            if (c < 0x20 && c != '\t')
                return line;
        return prefix + plain;
    }
    catch (...) { return line; }
}

// ─────────────────────────────────────────────────────────────────────────────
// receiveLoop
// ─────────────────────────────────────────────────────────────────────────────
static void receiveLoop(int fd, std::atomic<bool> &running)
{
    while (running)
    {
        std::string line = recvLine(fd);
        if (line.empty())
        {
            printMsg("[disconnected from server]");
            running = false;
            break;
        }

        // ── DH key exchange ───────────────────────────────────────────────────
        if (line.rfind("DH_INIT ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 4);
            if (p.size() == 4)
            {
                const std::string &peer = p[1];
                long long theirPub  = std::stoll(p[2]);
                auto theirNonce     = base64Decode(p[3]);

                CryptoSession s;
                s.init();
                s.deriveKey(theirPub, theirNonce);
                s.save(peer);
                sessionStore[peer] = s;

                std::vector<uint8_t> ourPreXorNonce(16);
                for (int i = 0; i < 16; i++)
                    ourPreXorNonce[i] = s.nonce[i] ^ theirNonce[i];

                sendLine(fd, "DH_REPLY " + peer + " " +
                             std::to_string(s.pubKey) + " " +
                             base64Encode(ourPreXorNonce));

                printMsg("[crypto] Secure session established with " + peer);
            }
            continue;
        }

        if (line.rfind("DH_REPLY ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 4);
            if (p.size() == 4)
            {
                const std::string &peer = p[1];
                long long theirPub  = std::stoll(p[2]);
                auto theirNonce     = base64Decode(p[3]);

                if (sessionStore.count(peer))
                {
                    sessionStore[peer].deriveKey(theirPub, theirNonce);
                    sessionStore[peer].save(peer);
                    ui.target = peer;
                    ui.screen = Screen::DM;
                    printMsg("[crypto] Secure session established with " + peer);
                    printMsg("Now chatting with " + peer + ". Type /help for commands.");
                }
            }
            continue;
        }

        // ── File transfer ─────────────────────────────────────────────────────
        if (line.rfind("FILE_ACCEPTED", 0) == 0)
        {
            if (pendingFilePath.empty()) { printMsg("[file] Nothing to send"); continue; }
            std::ifstream file(pendingFilePath, std::ios::binary);
            if (!file) { printMsg("[file] Could not open file"); pendingFilePath.clear(); continue; }
            printMsg("[file] Sending " + pendingFilePath + "...");
            constexpr size_t CHUNK = 3 * 1024;
            std::vector<uint8_t> buf(CHUNK);
            while (file.read(reinterpret_cast<char *>(buf.data()), CHUNK) || file.gcount() > 0)
            {
                size_t got = static_cast<size_t>(file.gcount());
                buf.resize(got);
                sendLine(fd, "FILE_DATA " + base64Encode(buf));
                buf.resize(CHUNK);
            }
            sendLine(fd, "FILE_END");
            pendingFilePath.clear();
            printMsg("[file] Sent.");
            continue;
        }

        if (line.rfind("FILE_REJECTED", 0) == 0)
        {
            pendingFilePath.clear();
            printMsg("[file] Transfer rejected.");
            continue;
        }

        if (line.rfind("FILE_INCOMING ", 0) == 0)
        {
            auto parts = splitMessage(line, ' ', 3);
            if (parts.size() >= 3)
            {
                incomingFilename = parts[1];
                incomingFilesize = std::stoull(parts[2]);
                incomingFileData.clear();
                incomingFileData.reserve(incomingFilesize);
                printMsg("[file] Receiving " + incomingFilename + "...");
            }
            continue;
        }

        if (line.rfind("FILE_DATA ", 0) == 0)
        {
            auto chunk = base64Decode(line.substr(10));
            incomingFileData.insert(incomingFileData.end(), chunk.begin(), chunk.end());
            continue;
        }

        if (line.rfind("FILE_END", 0) == 0)
        {
            if (!incomingFilename.empty())
            {
                std::string dir = incomingSender.empty() ? "received" : incomingSender;
                std::filesystem::create_directories(dir);
                std::string savePath = dir + "/" +
                    std::filesystem::path(incomingFilename).filename().string();
                std::ofstream out(savePath, std::ios::binary | std::ios::trunc);
                if (out)
                    out.write(reinterpret_cast<const char *>(incomingFileData.data()),
                              static_cast<std::streamsize>(incomingFileData.size()));
                printMsg("[file] Saved to " + savePath);
                incomingFilename.clear();
                incomingFilesize = 0;
                incomingFileData.clear();
                incomingSender.clear();
            }
            continue;
        }

        if (line.rfind("FILE_OFFER ", 0) == 0)
        {
            auto parts = splitMessage(line, ' ', 4);
            if (parts.size() >= 4) incomingSender = parts[1];
            std::string fname = parts.size() >= 3 ? parts[2] : "?";
            std::string fsize = parts.size() >= 4 ? parts[3] : "?";
            printMsg("[!] " + incomingSender + " wants to send you " + fname +
                     " (" + fsize + " bytes)  \xe2\x86\x92  /accept or /reject");
            continue;
        }

        // ── Voice call ────────────────────────────────────────────────────────
        if (line.rfind("CALL_OFFER ", 0) == 0)
        {
            printMsg("[!] Incoming call from " + line.substr(11) + "  \xe2\x86\x92  /accept or /reject");
            continue;
        }

        if (line.rfind("CALL_ACCEPTED ", 0) == 0)
        {
            auto parts = splitMessage(line, ' ', 3);
            int peerPort = std::stoi(parts[2]);
            if (peerPort != 0)
            {
                voiceCall.start(parts[1], peerPort);
                sendLine(fd, "CALL_PORT " + std::to_string(voiceCall.udp.localPort));
            }
            else { voiceCall.peerIp = parts[1]; }
            printMsg("[voice] Call connected.");
            continue;
        }

        if (line.rfind("CALL_PEER_PORT ", 0) == 0)
        {
            auto parts = splitMessage(line, ' ', 3);
            voiceCall.peerIp   = parts[1];
            voiceCall.peerPort = std::stoi(parts[2]);
            continue;
        }

        if (line == "CALL_REJECTED") { printMsg("[voice] Call rejected."); continue; }
        if (line == "CALL_ENDED")    { voiceCall.stop(); printMsg("[voice] Call ended."); continue; }

        // ── Incoming DM ───────────────────────────────────────────────────────
        if (line.rfind("MSG_FROM ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 3);
            if (p.size() == 3)
            {
                const std::string &sender = p[1];
                std::string text = (sessionStore.count(sender) && sessionStore[sender].ready)
                    ? sessionStore[sender].decryptMsg(p[2]) : p[2];
                if (ui.screen != Screen::DM || ui.target != sender)
                    printMsg("[DM from " + sender + "] " + text);
                else
                    printMsg(sender + ": " + text);
            }
            continue;
        }

        // ── Incoming room message ─────────────────────────────────────────────
        if (line.rfind("MSG_ROOM ", 0) == 0 || line.rfind("ROOM_MSG ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 4);
            if (p.size() == 4)
            {
                const std::string &room   = p[1];
                const std::string &sender = p[2];
                std::string text = (sessionStore.count(room) && sessionStore[room].ready)
                    ? sessionStore[room].decryptMsg(p[3]) : p[3];
                if (ui.screen != Screen::ROOM || ui.target != room)
                    printMsg("[" + room + "] " + sender + ": " + text);
                else
                    printMsg(sender + ": " + text);
            }
            continue;
        }

        // ── Auth responses ────────────────────────────────────────────────────
        if (line == "FOUND")
        {
            // Username exists → ask for password (no echo)
            ui.signingUp    = false;
            ui.authStep     = AuthStep::PASSWORD;
            ui.passwordMode = true;
            std::lock_guard<std::mutex> lock(printMtx);
            std::cout << "\r\033[K" << "Password: " << std::flush;
            continue;
        }

        if (line == "NOT_FOUND")
        {
            // Username not found → ask whether to create account
            ui.authStep = AuthStep::CONFIRM_SIGNUP;
            printMsg("Username '" + ui.pendingUsername + "' not found. Create account? (y/n): ");
            continue;
        }

        // ── Server OK / ERR / INFO ────────────────────────────────────────────
        if (line.rfind("OK Logged in as ", 0) == 0)
        {
            ui.username     = line.substr(16);
            ui.screen       = Screen::MENU;
            ui.passwordMode = false;
            disableRawMode();
            showMenu();
            enableRawMode();
            continue;
        }

        if (line.rfind("OK Logged out", 0) == 0)
        {
            ui.username.clear();
            ui.target.clear();
            ui.historyPeer.clear();
            ui.screen       = Screen::AUTH;
            ui.authStep     = AuthStep::USERNAME;
            ui.passwordMode = false;
            disableRawMode();
            showAuthScreen();
            enableRawMode();
            continue;
        }

        if (line.rfind("OK Joined ", 0) == 0) continue;

        if (line.rfind("INFO ", 0) == 0)
        {
            std::string content = line.substr(5);
            if (!ui.historyPeer.empty() && !content.empty() && content[0] == '[')
                content = tryDecryptHistoryLine(content);
            else
                ui.historyPeer.clear();
            printMsg(content);
            continue;
        }

        if (line.rfind("ERR ", 0) == 0)
        {
            // On auth error, reset to USERNAME step so user can try again
            if (ui.screen == Screen::AUTH)
            {
                ui.authStep     = AuthStep::USERNAME;
                ui.passwordMode = false;
                printMsg("[!] " + line.substr(4));
                std::lock_guard<std::mutex> lock(printMtx);
                std::cout << "Enter username: " << std::flush;
            }
            else { printMsg("[!] " + line.substr(4)); }
            continue;
        }
        if (line.rfind("OK ", 0) == 0) { printMsg(line.substr(3)); continue; }
        if (line != "OK")              { printMsg(line);            continue; }
    }
}

// ── readLine ──────────────────────────────────────────────────────────────────
// When ui.passwordMode is true, characters are not echoed.
static std::string readLine()
{
    inputBuf.clear();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (c == '\n' || c == '\r')
        {
            std::cout << "\n" << std::flush;
            std::string result = inputBuf;
            inputBuf.clear();
            return result;
        }
        if (c == 127 || c == '\b')
        {
            if (!inputBuf.empty())
            {
                inputBuf.pop_back();
                if (!ui.passwordMode)
                    std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        if (c == 3) { disableRawMode(); std::cout << "\n"; exit(0); }
        inputBuf += c;
        if (!ui.passwordMode)
            std::cout << c << std::flush;
    }
    return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// runClient
// ─────────────────────────────────────────────────────────────────────────────
void runClient(int fd)
{
    clientFd = fd;
    enableRawMode();
    showAuthScreen();

    std::atomic<bool> running{true};
    std::thread receiver(receiveLoop, fd, std::ref(running));

    while (running)
    {
        std::string line = readLine();
        if (line.empty() && ui.authStep != AuthStep::PASSWORD)
            continue;

        // ── AUTH ──────────────────────────────────────────────────────────────
        if (ui.screen == Screen::AUTH)
        {
            if (ui.authStep == AuthStep::USERNAME)
            {
                if (line.empty()) { std::cout << "Enter username: " << std::flush; continue; }
                ui.pendingUsername = line;
                sendLine(fd, "CHECK_USER " + line);
                continue;
            }

            if (ui.authStep == AuthStep::CONFIRM_SIGNUP)
            {
                if (line == "y" || line == "Y")
                {
                    ui.signingUp    = true;
                    ui.authStep     = AuthStep::PASSWORD;
                    ui.passwordMode = true;
                    std::lock_guard<std::mutex> lock(printMtx);
                    std::cout << "\r\033[K" << "Choose password: " << std::flush;
                }
                else if (line == "n" || line == "N")
                {
                    ui.authStep = AuthStep::USERNAME;
                    std::lock_guard<std::mutex> lock(printMtx);
                    std::cout << "\r\033[K" << "Enter username: " << std::flush;
                }
                else
                {
                    std::lock_guard<std::mutex> lock(printMtx);
                    std::cout << "\r\033[K" << "Enter y or n: " << std::flush;
                }
                continue;
            }

            if (ui.authStep == AuthStep::PASSWORD)
            {
                ui.passwordMode = false;
                if (ui.signingUp)
                    sendLine(fd, "SIGNUP " + ui.pendingUsername + " " + line);
                else
                    sendLine(fd, "LOGIN "  + ui.pendingUsername + " " + line);
                // authStep reset to USERNAME happens on ERR (failed) or
                // screen transitions to MENU (success) in receiveLoop
                continue;
            }
        }

        // ── MENU ──────────────────────────────────────────────────────────────
        if (ui.screen == Screen::MENU)
        {
            auto parts = splitMessage(line, ' ', 3);
            if (parts.empty()) continue;
            std::string cmd = parts[0];

            if (cmd == "dm" && parts.size() >= 2)
            {
                const std::string &peer = parts[1];
                if (!sessionStore.count(peer) || !sessionStore[peer].ready)
                {
                    CryptoSession s;
                    if (s.load(peer)) sessionStore[peer] = s;
                }
                if (sessionStore.count(peer) && sessionStore[peer].ready)
                {
                    ui.target = peer;
                    ui.screen = Screen::DM;
                    disableRawMode();
                    std::cout << "\033[2J\033[H";
                    std::cout << "Chatting with " << peer << ". Type /help for commands.\n\n";
                    std::cout << prompt() << std::flush;
                    enableRawMode();
                }
                else
                {
                    CryptoSession s;
                    s.init();
                    sessionStore[peer] = s;
                    sendLine(fd, "DH_INIT " + peer + " " +
                                 std::to_string(s.pubKey) + " " + base64Encode(s.nonce));
                    printMsg("Initiating secure session with " + peer + "...");
                }
            }
            else if (cmd == "join" && parts.size() >= 2)
            {
                ui.target = parts[1];
                ui.screen = Screen::ROOM;
                sendLine(fd, "JOIN " + parts[1]);
                disableRawMode();
                std::cout << "\033[2J\033[H";
                std::cout << "Joined " << parts[1] << ". Type /help for commands.\n\n";
                std::cout << prompt() << std::flush;
                enableRawMode();
            }
            else if (cmd == "create" && parts.size() >= 2) sendLine(fd, "CREATE_ROOM " + parts[1]);
            else if (cmd == "users")                        sendLine(fd, "LIST_USERS");
            else if (cmd == "rooms")                        sendLine(fd, "LIST_ROOMS");
            else if (cmd == "history" && parts.size() >= 3)
            {
                if      (parts[1] == "dm")   { ui.historyPeer = parts[2]; sendLine(fd, "HISTORY_DM "   + parts[2]); }
                else if (parts[1] == "room")                               sendLine(fd, "HISTORY_ROOM " + parts[2]);
            }
            else if (cmd == "logout")                { sendLine(fd, "LOGOUT"); }
            else if (cmd == "quit" || cmd == "exit") { break; }
            else printMsg("Unknown command. Try: dm, join, create, users, rooms, history, logout");
            continue;
        }

        // ── DM / ROOM ─────────────────────────────────────────────────────────
        if (ui.screen == Screen::DM || ui.screen == Screen::ROOM)
        {
            if (line == "/q")
            {
                if (ui.screen == Screen::ROOM) sendLine(fd, "LEAVE " + ui.target);
                ui.screen = Screen::MENU;
                ui.target.clear();
                disableRawMode();
                showMenu();
                enableRawMode();
                continue;
            }

            if (line == "/help")    { showChatHelp(); continue; }
            if (line == "/call")    { sendLine(fd, "CALL " + ui.target); continue; }
            if (line == "/endcall") { voiceCall.stop(); sendLine(fd, "CALL_END"); continue; }

            if (line == "/history")
            {
                if (ui.screen == Screen::DM) { ui.historyPeer = ui.target; sendLine(fd, "HISTORY_DM "   + ui.target); }
                else                                                         sendLine(fd, "HISTORY_ROOM " + ui.target);
                continue;
            }

            if (line.rfind("/send ", 0) == 0)
            {
                std::string path = line.substr(6);
                if (!std::filesystem::exists(path))
                    printMsg("[!] File not found: " + path);
                else
                {
                    size_t sz = std::filesystem::file_size(path);
                    pendingFilePath = path;
                    sendLine(fd, "FILE_SEND " + ui.target + " " +
                                 std::filesystem::path(path).filename().string() + " " +
                                 std::to_string(sz));
                }
                continue;
            }

            if (line == "/accept")
            {
                if (!incomingSender.empty()) sendLine(fd, "FILE_ACCEPT");
                else { voiceCall.start("", 0); sendLine(fd, "CALL_ACCEPT " + std::to_string(voiceCall.udp.localPort)); }
                continue;
            }

            if (line == "/reject")
            {
                if (!incomingSender.empty()) sendLine(fd, "FILE_REJECT");
                else                         sendLine(fd, "CALL_REJECT");
                continue;
            }

            std::string toSend = (sessionStore.count(ui.target) && sessionStore[ui.target].ready)
                ? sessionStore[ui.target].encryptMsg(line) : line;
            sendLine(fd, (ui.screen == Screen::DM ? "DM " : "MSG ") + ui.target + " " + toSend);
            printMsg("you: " + line);
            continue;
        }
    }

    running = false;
    disableRawMode();
    shutdown(fd, SHUT_RDWR);
    close(fd);
    receiver.join();
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char *argv[])
{
    std::string host = "127.0.0.1";
    int port = 8080;
    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);

    int fd = clientConnect(host, port);
    if (fd < 0)
    {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        return 1;
    }

    runClient(fd);
    return 0;
}