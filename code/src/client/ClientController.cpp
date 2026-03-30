#include "ClientController.h"
#include <iostream>
#include <filesystem>
#include <unistd.h>

ClientController::ClientController(int fd)
    : terminal(ui), net(fd), crypto(fd, sessionStore, ui.username), file(fd), voice(fd) {}

bool ClientController::isValidUsername(const std::string &username)
{
    if (username.empty())
        return false;
    for (char c : username)
        if (!isalnum(static_cast<unsigned char>(c)))
            return false;
    return true;
}

std::string ClientController::tryDecryptHistoryLine(const std::string &line)
{
    if (ui.historyPeer.empty())
        return line;
    return crypto.decrypt(ui.historyPeer, line);
}

void ClientController::run()
{
    terminal.enableRawMode();
    terminal.showAuthScreen();

    net.start([this](const std::string &line)
              { onServerMessage(line); });

    while (running)
    {
        // Don't let the user type if we are waiting for FOUND/NOT_FOUND/OK
        if (ui.waiting)
        {
            usleep(50000); // 50ms sleep to save CPU
            continue;
        }

        std::string line = terminal.readLine();
        if (line.empty() && ui.authStep != AuthStep::PASSWORD)
            continue;

        handleInput(line);
    }

    net.stop();
    terminal.disableRawMode();
}

void ClientController::onServerMessage(const std::string &line)
{
    ui.waiting = false;

    // 1. Auth Responses
    if (line == "FOUND")
    {
        if (ui.signingUp)
        {
            terminal.printMsg(BOLD RED + std::string("[!] ") + RESET + "Username '" + ui.pendingUsername + "' already exists. Try another username (or type 'back'):");
        }
        else
        {
            ui.authStep = AuthStep::PASSWORD;
            terminal.setPasswordMode(true);
            terminal.printMsg("Password (or type 'back'): ");
        }
        return;
    }

    if (line == "NOT_FOUND")
    {
        if (ui.signingUp)
        {
            ui.authStep = AuthStep::CONFIRM_SIGNUP;
            terminal.printMsg("Username '" + ui.pendingUsername + "' available. Create account? (y/n) or type 'back': ");
        }
        else
        {
            terminal.printMsg(BOLD RED + std::string("[!] ") + RESET + "User '" + ui.pendingUsername + "' not found. Try another username (or type 'back'):");
        }
        return;
    }

    // 2. Account / Login Status
    if (line.rfind("OK Signup successful", 0) == 0)
    {
        ui.authStep = AuthStep::CHOOSE_MODE;
        ui.signingUp = false;
        terminal.printMsg(BOLD GREEN + std::string("OK") + RESET + line.substr(2));
        terminal.showAuthScreen();
        return;
    }

    if (line.rfind("OK Logged in as ", 0) == 0)
    {
        ui.username = line.substr(16);
        ui.screen = Screen::MENU;
        terminal.setPasswordMode(false);
        terminal.disableRawMode();
        terminal.printMsg(BOLD GREEN + std::string("OK") + RESET + line.substr(2));
        terminal.showMenu();
        terminal.enableRawMode();
        return;
    }

    if (line.rfind("OK Logged out", 0) == 0)
    {
        ui.username.clear();
        ui.target.clear();
        ui.screen = Screen::AUTH;
        ui.authStep = AuthStep::CHOOSE_MODE;
        terminal.setPasswordMode(false);
        terminal.disableRawMode();
        terminal.printMsg(BOLD GREEN + std::string("OK") + RESET + line.substr(2));
        terminal.showAuthScreen();
        terminal.enableRawMode();
        return;
    }

    // 3. Room Management Responses
    if (line.rfind("OK Joined ", 0) == 0)
    {
        ui.target = line.substr(10);
        ui.screen = Screen::ROOM;
        terminal.disableRawMode();
        std::cout << "\033[2J\033[H" << "Joined " << BOLD YELLOW << ui.target << RESET << ". Type /help for commands.\n\n"
                  << terminal.prompt() << std::flush;
        terminal.enableRawMode();
        return;
    }

    if (line.rfind("OK Room created: ", 0) == 0)
    {
        ui.target = line.substr(17);
        ui.screen = Screen::ROOM;
        terminal.disableRawMode();
        std::cout << "\033[2J\033[H" << "Created and joined " << BOLD YELLOW << ui.target << RESET << ". Type /help for commands.\n\n"
                  << terminal.prompt() << std::flush;
        terminal.enableRawMode();
        return;
    }

    // 4. Handle Errors
    if (line.rfind("ERR ", 0) == 0)
    {
        terminal.printMsg(BOLD RED + std::string("ERR") + RESET + line.substr(3));
        if (ui.screen == Screen::AUTH)
        {
            if (line == "ERR Invalid username or password")
            {
                ui.authStep = AuthStep::PASSWORD;
                terminal.setPasswordMode(true);
                terminal.printMsg("Try again (or type 'back'): ");
            }
            else
            {
                ui.authStep = AuthStep::CHOOSE_MODE;
                terminal.setPasswordMode(false);
                terminal.showAuthScreen();
            }
        }
        return;
    }

    // 5. Handle Crypto DH Exchange
    if (crypto.handleDHReply(line) || crypto.handleDHInit(line))
    {
        auto p = splitMessage(line, ' ', 4);
        terminal.printMsg(BOLD BLUE + std::string("[crypto]") + RESET + " Secure session established with " + p[1]);
        return;
    }

    // 6. Handle Incoming Text Messages (DMs and Rooms)
    if (line.rfind("MSG_FROM ", 0) == 0)
    {
        auto p = splitMessage(line, ' ', 3);
        if (p.size() == 3)
        {
            std::string text = crypto.decrypt(p[1], p[2]);
            terminal.printMsg(BOLD YELLOW + p[1] + RESET + ": " + text);
        }
        return;
    }

    if (line.rfind("ROOM_MSG ", 0) == 0)
    {
        auto p = splitMessage(line, ' ', 4);
        if (p.size() == 4)
        {
            // p[1] is roomName, p[2] is sender, p[3] is content
            terminal.printMsg(BOLD MAGENTA "[" + p[1] + "] " + BOLD YELLOW + p[2] + RESET + ": " + p[3]);
        }
        return;
    }

    // 7. Handle Voice Routing
    std::string callSender;
    if (voice.handleCallOffer(line, callSender)) {
        terminal.printMsg(BOLD YELLOW + std::string("[!] ") + RESET + "Incoming call from " + callSender + "  \xe2\x86\x92  /accept or /reject");
        return;
    }

    std::string outIp;
    int outPort;
    if (voice.handleCallAccepted(line, outIp, outPort)) {
        if (outPort != 0) { 
            net.send("CALL_PORT " + std::to_string(voice.getLocalPort()));
        }
        terminal.printMsg(BOLD GREEN + std::string("[voice] ") + RESET + "Call connected.");
        return;
    }

    if (voice.handleCallPeerPort(line, outIp, outPort)) return;

    if (voice.handleCallRejected(line)) {
        terminal.printMsg(BOLD RED + std::string("[voice] ") + RESET + "Call rejected.");
        return;
    }

    if (voice.handleCallEnded(line)) {
        terminal.printMsg(BOLD BLUE + std::string("[voice] ") + RESET + "Call ended.");
        return;
    }

    // 8. Handle File Routing
    std::string fSender, fName, fSize, savedPath;
    if (file.handleFileOffer(line, fSender, fName, fSize)) {
        terminal.printMsg(BOLD YELLOW + std::string("[!] ") + RESET + fSender + " wants to send you " + fName + " (" + fSize + " bytes) \xe2\x86\x92 /accept or /reject");
        return;
    }
    
    if (file.handleFileAccepted(line)) {
        terminal.printMsg(BOLD BLUE + std::string("[file] ") + RESET + "Sending file...");
        return;
    }

    if (file.handleFileRejected(line)) {
        terminal.printMsg(BOLD RED + std::string("[file] ") + RESET + "Transfer rejected.");
        return;
    }

    if (file.handleFileIncoming(line)) {
        terminal.printMsg(BOLD BLUE + std::string("[file] ") + RESET + "Receiving file...");
        return;
    }

    if (file.handleFileData(line)) return;

    if (file.handleFileEnd(line, savedPath)) {
        if (!savedPath.empty()) 
            terminal.printMsg(BOLD GREEN + std::string("[file] ") + RESET + "Saved to " + savedPath);
        else 
            terminal.printMsg(BOLD GREEN + std::string("[file] ") + RESET + "Sent successfully.");
        return;
    }

    // 9. Informational / Generic Messages
    if (line.rfind("INFO ", 0) == 0)
    {
        std::string content = line.substr(5);
        if (!ui.historyPeer.empty() && !content.empty() && content[0] == '[')
            content = tryDecryptHistoryLine(content);
        else
            ui.historyPeer.clear();

        terminal.printMsg(BOLD BLUE + std::string("INFO ") + RESET + content);
        return;
    }

    if (line != "OK")
    {
        if (line.rfind("ERR ", 0) == 0)
            terminal.printMsg(BOLD RED + std::string("ERR") + RESET + line.substr(3));
        else if (line.rfind("[!] ", 0) == 0)
            terminal.printMsg(BOLD RED + std::string("[!] ") + RESET + line.substr(4));
        else
            terminal.printMsg(line);
    }
}

void ClientController::handleInput(const std::string &line)
{
    if (ui.screen == Screen::AUTH)
        handleAuthInput(line);
    else if (ui.screen == Screen::MENU)
        handleMenuInput(line);
    else if (ui.screen == Screen::DM || ui.screen == Screen::ROOM)
        handleChatInput(line);
}

void ClientController::handleAuthInput(const std::string &line)
{
    if (ui.authStep != AuthStep::CHOOSE_MODE && (line == "back" || line == "BACK" || line == "/back"))
    {
        ui.authStep = AuthStep::CHOOSE_MODE;
        terminal.setPasswordMode(false);
        terminal.showAuthScreen();
        return;
    }

    if (ui.authStep == AuthStep::CHOOSE_MODE)
    {
        if (line == "1" || line == "login" || line == "LOGIN")
        {
            ui.signingUp = false;
            ui.authStep = AuthStep::USERNAME;
            terminal.showAuthScreen();
        }
        else if (line == "2" || line == "signup" || line == "SIGNUP")
        {
            ui.signingUp = true;
            ui.authStep = AuthStep::USERNAME;
            terminal.showAuthScreen();
        }
        else
        {
            terminal.printMsg(BOLD RED + std::string("[!] ") + RESET + "Invalid choice. Please try again (type 1 or 2).");
            terminal.showAuthScreen();
        }
        return;
    }

    if (ui.authStep == AuthStep::USERNAME)
    {
        if (!isValidUsername(line))
        {
            terminal.printMsg(BOLD RED + std::string("[!] ") + RESET + "Username may only contain letters and numbers");
            return;
        }
        ui.pendingUsername = line;
        ui.waiting = true;
        net.send("CHECK_USER " + line);
        return;
    }

    if (ui.authStep == AuthStep::CONFIRM_SIGNUP)
    {
        if (line == "y" || line == "Y")
        {
            ui.authStep = AuthStep::PASSWORD;
            terminal.setPasswordMode(true);
            terminal.printMsg("Choose password (or type 'back'): ");
        }
        else
        {
            ui.authStep = AuthStep::CHOOSE_MODE;
            terminal.showAuthScreen();
        }
        return;
    }

    if (ui.authStep == AuthStep::PASSWORD)
    {
        terminal.setPasswordMode(false);
        ui.waiting = true; 
        if (ui.signingUp)
            net.send("SIGNUP " + ui.pendingUsername + " " + line);
        else
            net.send("LOGIN " + ui.pendingUsername + " " + line);
        return;
    }
}

void ClientController::handleMenuInput(const std::string &line)
{
    auto parts = splitMessage(line, ' ', 3);
    if (parts.empty())
        return;
    std::string cmd = parts[0];

    if (cmd == "dm" && parts.size() >= 2)
    {
        const std::string &peer = parts[1];
        ui.target = peer;
        if (crypto.isSessionReady(peer))
        {
            ui.screen = Screen::DM;
            terminal.disableRawMode();
            std::cout << "\033[2J\033[H" << "Chatting with " << BOLD YELLOW << peer << RESET << ". Type /help for commands.\n\n"
                      << terminal.prompt() << std::flush;
            terminal.enableRawMode();
        }
        else
        {
            crypto.initiateSession(peer);
            terminal.printMsg(BOLD BLUE + std::string("INFO ") + RESET + "Initiating secure session with " + peer + "...");
        }
    }
    else if (cmd == "join" && parts.size() >= 2)
    {
        // Wait for server to confirm via OK Joined before switching UI
        std::string command = "JOIN " + parts[1];
        if (parts.size() >= 3) command += " " + parts[2];
        net.send(command);
    }
    else if (cmd == "join" && parts.size() < 2)
    {
        terminal.printMsg(BOLD RED + std::string("ERR ") + RESET + "Usage: join <room> [password]");
    }
    else if (cmd == "create" && parts.size() >= 2)
    {
        std::string command = "CREATE_ROOM " + parts[1];
        if (parts.size() >= 3) command += " " + parts[2];
        net.send(command);
    }
    else if (cmd == "create" && parts.size() < 2)
    {
        terminal.printMsg(BOLD RED + std::string("ERR ") + RESET + "Usage: create <room> [password]");
    }
    else if (cmd == "users")
        net.send("LIST_USERS");
    else if (cmd == "rooms")
        net.send("LIST_ROOMS");
    else if (cmd == "history" && parts.size() >= 3)
    {
        if (parts[1] == "dm")
        {
            crypto.initiateSession(parts[2]); 
            ui.historyPeer = parts[2];
            net.send("HISTORY_DM " + parts[2]);
        }
        else if (parts[1] == "room")
            net.send("HISTORY_ROOM " + parts[2]);
    }
    else if (cmd == "logout")
        net.send("LOGOUT");
    else if (cmd == "quit" || cmd == "exit")
        running = false;
    else
        terminal.printMsg("Unknown command. Try: dm, join, create, users, rooms, history, logout");
}

void ClientController::handleChatInput(const std::string &line)
{
    if (line == "/q")
    {
        if (ui.screen == Screen::ROOM)
            net.send("LEAVE " + ui.target);
        ui.screen = Screen::MENU;
        ui.target.clear();
        terminal.disableRawMode();
        terminal.showMenu();
        terminal.enableRawMode();
        return;
    }
    if (line == "/help")
    {
        terminal.showChatHelp();
        return;
    }
    if (line == "/call")
    {
        net.send("CALL " + ui.target);
        return;
    }
    if (line == "/endcall")
    {
        voice.stopCall();
        net.send("CALL_END");
        return;
    }
    if (line == "/history")
    {
        if (ui.screen == Screen::DM)
        {
            crypto.initiateSession(ui.target); 
            ui.historyPeer = ui.target;
            net.send("HISTORY_DM " + ui.target);
        }
        else
            net.send("HISTORY_ROOM " + ui.target);
        return;
    }
    if (line.rfind("/send ", 0) == 0)
    {
        std::string path = line.substr(6);
        if (!std::filesystem::exists(path))
            terminal.printMsg(BOLD RED + std::string("[!] ") + RESET + "File not found: " + path);
        else
        {
            size_t sz = std::filesystem::file_size(path);
            file.setPendingPath(path);
            net.send("FILE_SEND " + ui.target + " " + std::filesystem::path(path).filename().string() + " " + std::to_string(sz));
        }
        return;
    }
    if (line == "/accept")
    {
        if (!file.getIncomingSender().empty())
            net.send("FILE_ACCEPT");
        else
        {
            voice.startCall("", 0);
            net.send("CALL_ACCEPT " + std::to_string(voice.getLocalPort()));
        }
        return;
    }
    if (line == "/reject")
    {
        if (!file.getIncomingSender().empty())
            net.send("FILE_REJECT");
        else
            net.send("CALL_REJECT");
        return;
    }
    
    // Default: Send standard message
    std::string toSend = crypto.encrypt(ui.target, line);
    net.send((ui.screen == Screen::DM ? "DM " : "MSG ") + ui.target + " " + toSend);
    terminal.printMsg(BOLD GREEN + std::string("you") + RESET + ": " + line);
}