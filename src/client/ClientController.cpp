#include "ClientController.hpp"
#include <iostream>
#include <filesystem>
#include <unistd.h>

ClientController::ClientController(int fd)
    : terminal(ui), net(fd), crypto(fd, sessionStore), file(fd), voice(fd) {}

bool ClientController::isValidUsername(const std::string &username) {
    if (username.empty()) return false;
    for (char c : username)
        if (!isalnum(static_cast<unsigned char>(c)))
            return false;
    return true;
}

std::string ClientController::tryDecryptHistoryLine(const std::string &line) {
    if (ui.historyPeer.empty()) return line;
    return crypto.decrypt(ui.historyPeer, line);
}

void ClientController::run() {
    terminal.enableRawMode();
    terminal.showAuthScreen();

    net.start([this](const std::string& line) {
        onServerMessage(line);
    });

    while (running) {
        std::string line = terminal.readLine();
        if (line.empty() && ui.authStep != AuthStep::PASSWORD)
            continue;
        handleInput(line);
    }

    net.stop();
    terminal.disableRawMode();
}

void ClientController::onServerMessage(const std::string& line) {
    if (line == "[disconnected from server]") {
        terminal.printMsg(line);
        running = false;
        return;
    }

    if (crypto.handleDHInit(line)) {
        auto p = splitMessage(line, ' ', 4);
        terminal.printMsg("[crypto] Secure session established with " + p[1]);
        return;
    }

    if (crypto.handleDHReply(line)) {
        auto p = splitMessage(line, ' ', 4);
        ui.target = p[1];
        ui.screen = Screen::DM;
        terminal.printMsg("[crypto] Secure session established with " + p[1]);
        terminal.printMsg("Now chatting with " + p[1] + ". Type /help for commands.");
        return;
    }

    if (file.handleFileAccepted(line)) {
        terminal.printMsg("[file] Sending " + file.getPendingPath() + "...");
        // Sending is handled inside FileHandler::handleFileAccepted (blocking loop)
        terminal.printMsg("[file] Sent.");
        return;
    }

    if (file.handleFileRejected(line)) {
        terminal.printMsg("[file] Transfer rejected.");
        return;
    }

    if (file.handleFileIncoming(line)) {
        auto parts = splitMessage(line, ' ', 3);
        terminal.printMsg("[file] Receiving " + parts[1] + "...");
        return;
    }

    if (file.handleFileData(line)) return;

    std::string savedPath;
    if (file.handleFileEnd(line, savedPath)) {
        if (!savedPath.empty()) terminal.printMsg("[file] Saved to " + savedPath);
        return;
    }

    std::string sender, fname, fsize;
    if (file.handleFileOffer(line, sender, fname, fsize)) {
        terminal.printMsg("[!] " + sender + " wants to send you " + fname +
                          " (" + fsize + " bytes)  →  /accept or /reject");
        return;
    }

    if (voice.handleCallOffer(line, sender)) {
        terminal.printMsg("[!] Incoming call from " + sender + "  →  /accept or /reject");
        return;
    }

    std::string ip; int port;
    if (voice.handleCallAccepted(line, ip, port)) {
        if (port != 0) {
            net.send("CALL_PORT " + std::to_string(voice.getLocalPort()));
        }
        terminal.printMsg("[voice] Call connected.");
        return;
    }

    if (voice.handleCallPeerPort(line, ip, port)) return;
    if (voice.handleCallRejected(line)) { terminal.printMsg("[voice] Call rejected."); return; }
    if (voice.handleCallEnded(line)) { terminal.printMsg("[voice] Call ended."); return; }

    if (line.rfind("MSG_FROM ", 0) == 0) {
        auto p = splitMessage(line, ' ', 3);
        if (p.size() == 3) {
            const std::string &sender = p[1];
            std::string text = crypto.decrypt(sender, p[2]);
            if (ui.screen != Screen::DM || ui.target != sender)
                terminal.printMsg("[DM from " + sender + "] " + text);
            else
                terminal.printMsg(sender + ": " + text);
        }
        return;
    }

    if (line.rfind("MSG_ROOM ", 0) == 0 || line.rfind("ROOM_MSG ", 0) == 0) {
        auto p = splitMessage(line, ' ', 4);
        if (p.size() == 4) {
            const std::string &room   = p[1];
            const std::string &sender = p[2];
            std::string text = crypto.decrypt(room, p[3]);
            if (ui.screen != Screen::ROOM || ui.target != room)
                terminal.printMsg("[" + room + "] " + sender + ": " + text);
            else
                terminal.printMsg(sender + ": " + text);
        }
        return;
    }

    if (line == "FOUND") {
        ui.signingUp = false;
        ui.authStep = AuthStep::PASSWORD;
        terminal.setPasswordMode(true);
        std::cout << "\r\033[K" << "Password: " << std::flush;
        return;
    }

    if (line == "NOT_FOUND") {
        ui.authStep = AuthStep::CONFIRM_SIGNUP;
        terminal.printMsg("Username '" + ui.pendingUsername + "' not found. Create account? (y/n): ");
        return;
    }

    if (line.rfind("OK Logged in as ", 0) == 0) {
        ui.username = line.substr(16);
        ui.screen = Screen::MENU;
        terminal.setPasswordMode(false);
        terminal.disableRawMode();
        terminal.showMenu();
        terminal.enableRawMode();
        return;
    }

    if (line.rfind("OK Logged out", 0) == 0) {
        ui.username.clear(); ui.target.clear(); ui.historyPeer.clear();
        ui.screen = Screen::AUTH;
        ui.authStep = AuthStep::USERNAME;
        terminal.setPasswordMode(false);
        terminal.disableRawMode();
        terminal.showAuthScreen();
        terminal.enableRawMode();
        return;
    }

    if (line.rfind("OK Joined ", 0) == 0) return;

    if (line.rfind("INFO ", 0) == 0) {
        std::string content = line.substr(5);
        if (!ui.historyPeer.empty() && !content.empty() && content[0] == '[') {
            size_t closeBracket = content.find(']');
            if (closeBracket != std::string::npos) {
                size_t colonPos = content.find(':', closeBracket + 1);
                if (colonPos != std::string::npos) {
                    std::string metadata = content.substr(0, colonPos + 1);
                    std::string encrypted = content.substr(colonPos + 1);
                    while (!encrypted.empty() && encrypted.front() == ' ') encrypted.erase(0, 1);
                    std::string decrypted = crypto.decrypt(ui.historyPeer, encrypted);
                    content = metadata + " " + decrypted;
                } else {
                    ui.historyPeer.clear();
                }
            } else {
                ui.historyPeer.clear();
            }
        } else {
            ui.historyPeer.clear();
        }
        terminal.printMsg(content);
        return;
    }

    if (line.rfind("ERR ", 0) == 0) {
        if (ui.screen == Screen::AUTH) {
            ui.authStep = AuthStep::USERNAME;
            terminal.setPasswordMode(false);
            terminal.printMsg("[!] " + line.substr(4));
            std::cout << "Enter username: " << std::flush;
        } else {
            terminal.printMsg("[!] " + line.substr(4));
        }
        return;
    }

    if (line.rfind("OK ", 0) == 0) { terminal.printMsg(line.substr(3)); return; }
    if (line != "OK") { terminal.printMsg(line); return; }
}

void ClientController::handleInput(const std::string& line) {
    if (ui.screen == Screen::AUTH) handleAuthInput(line);
    else if (ui.screen == Screen::MENU) handleMenuInput(line);
    else if (ui.screen == Screen::DM || ui.screen == Screen::ROOM) handleChatInput(line);
}

void ClientController::handleAuthInput(const std::string& line) {
    if (ui.authStep == AuthStep::USERNAME) {
        if (line.empty()) { std::cout << "Enter username: " << std::flush; return; }
        if (!isValidUsername(line)) {
            terminal.printMsg("[!] Username may only contain letters and numbers");
            std::cout << "Enter username: " << std::flush;
            return;
        }
        ui.pendingUsername = line;
        net.send("CHECK_USER " + line);
        return;
    }
    if (ui.authStep == AuthStep::CONFIRM_SIGNUP) {
        if (line == "y" || line == "Y") {
            ui.signingUp = true; ui.authStep = AuthStep::PASSWORD;
            terminal.setPasswordMode(true);
            std::cout << "\r\033[K" << "Choose password: " << std::flush;
        } else if (line == "n" || line == "N") {
            ui.authStep = AuthStep::USERNAME;
            std::cout << "\r\033[K" << "Enter username: " << std::flush;
        } else {
            std::cout << "\r\033[K" << "Enter y or n: " << std::flush;
        }
        return;
    }
    if (ui.authStep == AuthStep::PASSWORD) {
        terminal.setPasswordMode(false);
        if (ui.signingUp) net.send("SIGNUP " + ui.pendingUsername + " " + line);
        else net.send("LOGIN " + ui.pendingUsername + " " + line);
        return;
    }
}

void ClientController::handleMenuInput(const std::string& line) {
    auto parts = splitMessage(line, ' ', 3);
    if (parts.empty()) return;
    std::string cmd = parts[0];

    if (cmd == "dm" && parts.size() >= 2) {
        const std::string &peer = parts[1];
        if (crypto.isSessionReady(peer)) {
            ui.target = peer; ui.screen = Screen::DM;
            terminal.disableRawMode();
            std::cout << "\033[2J\033[H" << "Chatting with " << peer << ". Type /help for commands.\n\n" << terminal.prompt() << std::flush;
            terminal.enableRawMode();
        } else {
            crypto.initiateSession(peer);
            terminal.printMsg("Initiating secure session with " + peer + "...");
        }
    } else if (cmd == "join" && parts.size() >= 3) {
        ui.target = parts[1]; ui.screen = Screen::ROOM;
        net.send("JOIN " + parts[1] + " " + parts[2]);
        terminal.disableRawMode();
        std::cout << "\033[2J\033[H" << "Joined " << parts[1] << ". Type /help for commands.\n\n" << terminal.prompt() << std::flush;
        terminal.enableRawMode();
    } else if (cmd == "join" && parts.size() == 2) {
        terminal.printMsg("ERR Usage: join <room> <password>");
    } else if (cmd == "create" && parts.size() >= 3) net.send("CREATE_ROOM " + parts[1] + " " + parts[2]);
    else if (cmd == "create" && parts.size() == 2) terminal.printMsg("ERR Usage: create <room> <password>");
    else if (cmd == "users") net.send("LIST_USERS");
    else if (cmd == "rooms") net.send("LIST_ROOMS");
    else if (cmd == "history" && parts.size() >= 3) {
        if (parts[1] == "dm") {
            crypto.initiateSession(parts[2]); // Ensure crypto session is loaded
            ui.historyPeer = parts[2];
            net.send("HISTORY_DM " + parts[2]);
        }
        else if (parts[1] == "room") net.send("HISTORY_ROOM " + parts[2]);
    } else if (cmd == "logout") net.send("LOGOUT");
    else if (cmd == "quit" || cmd == "exit") running = false;
    else terminal.printMsg("Unknown command. Try: dm, join, create, users, rooms, history, logout");
}

void ClientController::handleChatInput(const std::string& line) {
    if (line == "/q") {
        if (ui.screen == Screen::ROOM) net.send("LEAVE " + ui.target);
        ui.screen = Screen::MENU; ui.target.clear();
        terminal.disableRawMode();
        terminal.showMenu();
        terminal.enableRawMode();
        return;
    }
    if (line == "/help") { terminal.showChatHelp(); return; }
    if (line == "/call") { net.send("CALL " + ui.target); return; }
    if (line == "/endcall") { voice.stopCall(); net.send("CALL_END"); return; }
    if (line == "/history") {
        if (ui.screen == Screen::DM) {
            crypto.initiateSession(ui.target); // Ensure crypto session is loaded
            ui.historyPeer = ui.target;
            net.send("HISTORY_DM " + ui.target);
        }
        else net.send("HISTORY_ROOM " + ui.target);
        return;
    }
    if (line.rfind("/send ", 0) == 0) {
        std::string path = line.substr(6);
        if (!std::filesystem::exists(path)) terminal.printMsg("[!] File not found: " + path);
        else {
            size_t sz = std::filesystem::file_size(path);
            file.setPendingPath(path);
            net.send("FILE_SEND " + ui.target + " " + std::filesystem::path(path).filename().string() + " " + std::to_string(sz));
        }
        return;
    }
    if (line == "/accept") {
        if (!file.getIncomingSender().empty()) net.send("FILE_ACCEPT");
        else {
            voice.startCall("", 0);
            net.send("CALL_ACCEPT " + std::to_string(voice.getLocalPort()));
        }
        return;
    }
    if (line == "/reject") {
        if (!file.getIncomingSender().empty()) net.send("FILE_REJECT");
        else net.send("CALL_REJECT");
        return;
    }
    std::string toSend = crypto.encrypt(ui.target, line);
    net.send((ui.screen == Screen::DM ? "DM " : "MSG ") + ui.target + " " + toSend);
    terminal.printMsg("you: " + line);
}
