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
#include "../voice/voice_call.hpp"

static VoiceCall voiceCall;

// ── Session store (one entry per peer username) ──────────────────────────────
std::unordered_map<std::string, CryptoSession> sessionStore;

// ── File transfer state ───────────────────────────────────────────────────────
static std::string pendingFilePath;
static std::string incomingFilename;
static std::string incomingSender;
static size_t incomingFilesize = 0;
static std::vector<uint8_t> incomingFileData;

// ── Chat mode ─────────────────────────────────────────────────────────────────
enum class ChatMode
{
    NORMAL,
    DM,
    ROOM
};
static ChatMode chatMode = ChatMode::NORMAL;
static std::string chatTarget; // peer username (DM) or room name (ROOM)

// Forward declaration — receiveLoop needs to call sendLine
static int clientFd = -1;

// ─────────────────────────────────────────────────────────────────────────────
// receiveLoop — runs on a background thread
// ─────────────────────────────────────────────────────────────────────────────
static void receiveLoop(int fd, std::atomic<bool> &running)
{
    while (running)
    {
        std::string line = recvLine(fd);
        if (line.empty())
        {
            std::cout << "\n[disconnected from server]\n";
            running = false;
            break;
        }

        // ── DH key exchange — initiated by a remote peer ─────────────────────
        // Server relays: "DH_INIT <from_user> <their_pubkey> <their_nonce_b64>"
        if (line.rfind("DH_INIT ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 4);
            if (p.size() == 4)
            {
                const std::string &peer = p[1];
                long long theirPub = std::stoll(p[2]);
                auto theirNonce = base64Decode(p[3]);

                CryptoSession s;
                s.init();
                s.deriveKey(theirPub, theirNonce);
                sessionStore[peer] = s;

                // Re-derive our pre-XOR nonce so the initiator can apply the same XOR
                std::vector<uint8_t> ourPreXorNonce(16);
                for (int i = 0; i < 16; i++)
                    ourPreXorNonce[i] = s.nonce[i] ^ theirNonce[i];

                sendLine(fd, "DH_REPLY " + peer + " " +
                                 std::to_string(s.pubKey) + " " +
                                 base64Encode(ourPreXorNonce));

                std::cout << "\r[crypto] Secure session established with " << peer << "\n";
            }

            if (chatMode == ChatMode::DM || chatMode == ChatMode::ROOM)
                std::cout << ">> " << std::flush;
            else
                std::cout << "> " << std::flush;
            continue;
        }

        // ── DH key exchange — reply to our own DH_INIT ───────────────────────
        // Server relays: "DH_REPLY <from_user> <their_pubkey> <their_nonce_b64>"
        if (line.rfind("DH_REPLY ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 4);
            if (p.size() == 4)
            {
                const std::string &peer = p[1];
                long long theirPub = std::stoll(p[2]);
                auto theirNonce = base64Decode(p[3]);

                if (sessionStore.count(peer))
                {
                    sessionStore[peer].deriveKey(theirPub, theirNonce);
                    std::cout << "\r[crypto] Secure session established with " << peer << "\n";

                    chatTarget = peer;
                    chatMode = ChatMode::DM;
                    std::cout << "[DM with " << peer
                              << "] Type messages, '/q' to exit.\n>> " << std::flush;
                }
            }
            continue;
        }

        // ── File transfer ─────────────────────────────────────────────────────
        if (line.rfind("FILE_ACCEPTED", 0) == 0)
        {
            if (pendingFilePath.empty())
            {
                std::cout << "\r[file] No pending file to send\n> " << std::flush;
                continue;
            }
            std::ifstream file(pendingFilePath, std::ios::binary);
            if (!file)
            {
                std::cout << "\r[file] Could not open file\n> " << std::flush;
                pendingFilePath.clear();
                continue;
            }
            std::cout << "\r[file] Sending " << pendingFilePath << "...\n> " << std::flush;

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
            std::cout << "\r[file] Transfer complete\n> " << std::flush;
            continue;
        }

        if (line.rfind("FILE_REJECTED", 0) == 0)
        {
            pendingFilePath.clear();
            std::cout << "\r[file] Transfer rejected\n> " << std::flush;
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
                std::cout << "\r[file] Receiving " << incomingFilename
                          << " (" << incomingFilesize << " bytes)...\n> " << std::flush;
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
                std::cout << "\r[file] Saved to " << savePath << "\n> " << std::flush;
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
            if (parts.size() >= 2)
                incomingSender = parts[1];
            std::cout << "\r" << line << "\n";
            std::cout << "  → Type 'FILE_ACCEPT' or 'FILE_REJECT'\n> " << std::flush;
            continue;
        }

        // ── Voice call ────────────────────────────────────────────────────────
        if (line.rfind("CALL_OFFER ", 0) == 0)
        {
            std::cout << "\r[voice] Incoming call from " << line.substr(11)
                      << "\n  → Type 'CALL_ACCEPT' or 'CALL_REJECT'\n> " << std::flush;
            continue;
        }

        if (line.rfind("CALL_ACCEPTED ", 0) == 0)
        {
            auto parts = splitMessage(line, ' ', 3);
            std::string peerIp = parts[1];
            int peerPort = std::stoi(parts[2]);

            if (peerPort != 0)
            {
                // We are the CALLER — start voice and report our UDP port
                voiceCall.start(peerIp, peerPort);
                sendLine(fd, "CALL_PORT " + std::to_string(voiceCall.udp.localPort));
            }
            else
            {
                // We are the RECEIVER — already started, just update peer IP
                voiceCall.peerIp = peerIp;
            }
            std::cout << "\r[voice] Call connected\n> " << std::flush;
            continue;
        }

        if (line.rfind("CALL_PEER_PORT ", 0) == 0)
        {
            auto parts = splitMessage(line, ' ', 3);
            voiceCall.peerIp = parts[1];
            voiceCall.peerPort = std::stoi(parts[2]);
            std::cout << "\r[voice] Peer ready\n> " << std::flush;
            continue;
        }

        if (line == "CALL_REJECTED")
        {
            std::cout << "\r[voice] Call rejected\n> " << std::flush;
            continue;
        }

        if (line == "CALL_ENDED")
        {
            voiceCall.stop();
            std::cout << "\r[voice] Call ended by peer\n> " << std::flush;
            continue;
        }

        // ── Incoming DM ───────────────────────────────────────────────────────
        // Format: "MSG_FROM <sender> <encrypted_content>"
        if (line.rfind("MSG_FROM ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 3);
            if (p.size() == 3)
            {
                const std::string &sender = p[1];
                if (sessionStore.count(sender) && sessionStore[sender].ready)
                {
                    std::string plaintext = sessionStore[sender].decryptMsg(p[2]);
                    std::cout << "\r[" << sender << "]: " << plaintext << "\n";
                }
                else
                {
                    std::cout << "\r[" << sender << " UNENCRYPTED]: " << p[2] << "\n";
                }
            }

            if (chatMode == ChatMode::DM || chatMode == ChatMode::ROOM)
                std::cout << ">> " << std::flush;
            else
                std::cout << "> " << std::flush;
            continue;
        }

        // ── Incoming room message ─────────────────────────────────────────────
        // Format: "MSG_ROOM <room> <sender> <encrypted_content>"
        if (line.rfind("MSG_ROOM ", 0) == 0)
        {
            auto p = splitMessage(line, ' ', 4);
            if (p.size() == 4)
            {
                const std::string &room = p[1];
                const std::string &sender = p[2];
                if (sessionStore.count(room) && sessionStore[room].ready)
                {
                    std::string plaintext = sessionStore[room].decryptMsg(p[3]);
                    std::cout << "\r[" << sender << " @ " << room << "]: "
                              << plaintext << "\n";
                }
                else
                {
                    std::cout << "\r[" << sender << " @ " << room
                              << " UNENCRYPTED]: " << p[3] << "\n";
                }
            }

            if (chatMode == ChatMode::DM || chatMode == ChatMode::ROOM)
                std::cout << ">> " << std::flush;
            else
                std::cout << "> " << std::flush;
            continue;
        }

        // ── All other server messages (INFO, OK, ERR, etc.) ───────────────────
        std::cout << "\r" << line << "\n";
        if (chatMode == ChatMode::DM || chatMode == ChatMode::ROOM)
            std::cout << ">> " << std::flush;
        else
            std::cout << "> " << std::flush;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// runClient — main input loop on the main thread
// ─────────────────────────────────────────────────────────────────────────────
void runClient(int fd)
{
    clientFd = fd;

    std::atomic<bool> running{true};
    std::thread receiver(receiveLoop, fd, std::ref(running));
    std::cout << "Connected. Type commands or 'quit' to exit.\n> " << std::flush;

    std::string line;
    while (running && std::getline(std::cin, line))
    {
        if (line == "quit" || line == "exit")
            break;

        // ── DM / ROOM sub-environment ─────────────────────────────────────────
        if (chatMode == ChatMode::DM || chatMode == ChatMode::ROOM)
        {
            if (line == "/q")
            {
                if (chatMode == ChatMode::ROOM)
                {
                    sendLine(fd, "LEAVE " + chatTarget);
                    std::cout << "[left room " << chatTarget << "]\n> " << std::flush;
                }
                else
                {
                    std::cout << "[left DM with " << chatTarget << "]\n> " << std::flush;
                }
                chatMode = ChatMode::NORMAL;
                chatTarget.clear();
                continue;
            }

            if (sessionStore.count(chatTarget) && sessionStore[chatTarget].ready)
            {
                std::string encrypted = sessionStore[chatTarget].encryptMsg(line);
                if (chatMode == ChatMode::DM)
                    sendLine(fd, "DM " + chatTarget + " " + encrypted);
                else
                    sendLine(fd, "MSG " + chatTarget + " " + encrypted);
            }
            else
            {
                std::cerr << "[crypto] No session ready for " << chatTarget
                          << " — message not sent\n";
            }

            std::cout << ">> " << std::flush;
            continue;
        }

        // ── Normal mode ───────────────────────────────────────────────────────
        auto parts = splitMessage(line, ' ', 3);
        if (parts.empty())
            continue;
        const std::string &cmd = parts[0];

        if (cmd == "DM" && parts.size() >= 2)
        {
            const std::string &peer = parts[1];

            if (sessionStore.count(peer) && sessionStore[peer].ready)
            {
                chatTarget = peer;
                chatMode = ChatMode::DM;
                std::cout << "[DM with " << peer
                          << "] Type messages, '/q' to exit.\n>> " << std::flush;
            }
            else
            {
                CryptoSession s;
                s.init();
                sessionStore[peer] = s;

                sendLine(fd, "DH_INIT " + peer + " " +
                                 std::to_string(s.pubKey) + " " +
                                 base64Encode(s.nonce));

                std::cout << "[crypto] Key exchange initiated with " << peer
                          << ", waiting for reply...\n> " << std::flush;
            }
            continue;
        }

        if (cmd == "JOIN" && parts.size() >= 2)
        {
            const std::string &room = parts[1];
            chatTarget = room;
            chatMode = ChatMode::ROOM;
            sendLine(fd, "JOIN " + room);
            std::cout << "[Room " << room
                      << "] Type messages, '/q' to exit.\n>> " << std::flush;
            continue;
        }

        if (cmd == "FILE_SEND" && parts.size() >= 3)
        {
            const std::string &username = parts[1];
            const std::string &filepath = parts[2];
            if (!std::filesystem::exists(filepath))
            {
                std::cerr << "File not found: " << filepath << "\n> " << std::flush;
                continue;
            }
            size_t filesize = std::filesystem::file_size(filepath);
            pendingFilePath = filepath;
            sendLine(fd, "FILE_SEND " + username + " " +
                             std::filesystem::path(filepath).filename().string() + " " +
                             std::to_string(filesize));
        }
        else if (cmd == "FILE_ACCEPT")
        {
            sendLine(fd, "FILE_ACCEPT");
        }
        else if (cmd == "FILE_REJECT")
        {
            sendLine(fd, "FILE_REJECT");
        }
        else if (cmd == "CALL_ACCEPT")
        {
            voiceCall.start("", 0);
            sendLine(fd, "CALL_ACCEPT " + std::to_string(voiceCall.udp.localPort));
        }
        else if (cmd == "CALL_END")
        {
            voiceCall.stop();
            sendLine(fd, "CALL_END");
        }
        else
        {
            // SIGNUP, LOGIN, LOGOUT, LIST_USERS, HISTORY_DM, etc.
            // Protocol commands — sent plaintext intentionally.
            sendLine(fd, line);
        }

        std::cout << "> " << std::flush;
    }

    running = false;
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

    if (argc >= 2)
        host = argv[1];
    if (argc >= 3)
        port = std::stoi(argv[2]);

    int fd = clientConnect(host, port);
    if (fd < 0)
    {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        return 1;
    }

    // No global handshake — keys are exchanged lazily on first DM with each peer
    runClient(fd);
    return 0;
}