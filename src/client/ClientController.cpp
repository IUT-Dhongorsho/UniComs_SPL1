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

    if (line == "FOUND")
    {
        if (ui.signingUp)
        {
            // Error: Trying to signup with existing name. Prompt for another instead of kicking to menu.
            terminal.printMsg("[!] Username '" + ui.pendingUsername + "' already exists. Try another username (or type 'back'):");
            // Stay in AuthStep::USERNAME
        }
        else
        {
            // Success: Logging in
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
            // Success: Name available for signup
            ui.authStep = AuthStep::CONFIRM_SIGNUP;
            terminal.printMsg("Username '" + ui.pendingUsername + "' available. Create account? (y/n) or type 'back': ");
        }
        else
        {
            // Error: Trying to login with non-existent name. Prompt for another instead of kicking to menu.
            terminal.printMsg("[!] User '" + ui.pendingUsername + "' not found. Try another username (or type 'back'):");
            // Stay in AuthStep::USERNAME
        }
        return;
    }

    // 4. Handle Successful Auth Transitions
    if (line.rfind("OK Signup successful", 0) == 0)
    {
        // Account created, now force user back to the Choose Mode screen to Login
        ui.authStep = AuthStep::CHOOSE_MODE;
        ui.signingUp = false;
        terminal.printMsg(line);
        terminal.showAuthScreen();
        return;
    }

    if (line.rfind("OK Logged in as ", 0) == 0)
    {
        ui.username = line.substr(16);
        ui.screen = Screen::MENU;
        terminal.setPasswordMode(false);
        terminal.disableRawMode();
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
        terminal.showAuthScreen();
        terminal.enableRawMode();
        return;
    }

    // 5. Handle Errors
    if (line.rfind("ERR ", 0) == 0)
    {
        terminal.printMsg("[!] " + line.substr(4));

        // If an error happens during Auth, allow retry or fallback gracefully
        if (ui.screen == Screen::AUTH)
        {
            if (line == "ERR Invalid username or password")
            {
                // Wrong password: Let them try again or go back
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

    // 6. Handle Messaging and Crypto (Existing Logic)
    if (crypto.handleDHReply(line) || crypto.handleDHInit(line))
    {
        auto p = splitMessage(line, ' ', 4);
        terminal.printMsg("[crypto] Secure session established with " + p[1]);
        return;
    }

    if (line.rfind("MSG_FROM ", 0) == 0)
    {
        auto p = splitMessage(line, ' ', 3);
        if (p.size() == 3)
        {
            std::string text = crypto.decrypt(p[1], p[2]);
            terminal.printMsg(p[1] + ": " + text);
        }
        return;
    }

    // 7. Informational / Generic Messages
    if (line.rfind("INFO ", 0) == 0)
    {
        terminal.printMsg(line.substr(5));
        return;
    }

    if (line != "OK")
    {
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
    // Global "back" handler for any step past the main menu
    if (ui.authStep != AuthStep::CHOOSE_MODE && (line == "back" || line == "BACK"))
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
            // Invalid choice handler
            terminal.printMsg("[!] Invalid choice. Please try again (type 1 or 2).");
            terminal.showAuthScreen();
        }
        return;
    }

    if (ui.authStep == AuthStep::USERNAME)
    {
        if (!isValidUsername(line))
        {
            terminal.printMsg("[!] Username may only contain letters and numbers");
            return;
        }
        ui.pendingUsername = line;
        ui.waiting = true; // <-- LOCK UI
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
        else // 'n' or anything else effectively acts as 'back'
        {
            ui.authStep = AuthStep::CHOOSE_MODE;
            terminal.showAuthScreen();
        }
        return;
    }

    if (ui.authStep == AuthStep::PASSWORD)
    {
        terminal.setPasswordMode(false);
        ui.waiting = true; // <-- LOCK UI while server verifies password
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
            std::cout << "\033[2J\033[H" << "Chatting with " << peer << ". Type /help for commands.\n\n"
                      << terminal.prompt() << std::flush;
            terminal.enableRawMode();
        }
        else
        {
            crypto.initiateSession(peer);
            terminal.printMsg("Initiating secure session with " + peer + "...");
        }
    }
    else if (cmd == "join" && parts.size() >= 3)
    {
        ui.target = parts[1];
        ui.screen = Screen::ROOM;
        net.send("JOIN " + parts[1] + " " + parts[2]);
        terminal.disableRawMode();
        std::cout << "\033[2J\033[H" << "Joined " << parts[1] << ". Type /help for commands.\n\n"
                  << terminal.prompt() << std::flush;
        terminal.enableRawMode();
    }
    else if (cmd == "join" && parts.size() == 2)
    {
        terminal.printMsg("ERR Usage: join <room> <password>");
    }
    else if (cmd == "create" && parts.size() >= 3)
        net.send("CREATE_ROOM " + parts[1] + " " + parts[2]);
    else if (cmd == "create" && parts.size() == 2)
        terminal.printMsg("ERR Usage: create <room> <password>");
    else if (cmd == "users")
        net.send("LIST_USERS");
    else if (cmd == "rooms")
        net.send("LIST_ROOMS");
    else if (cmd == "history" && parts.size() >= 3)
    {
        if (parts[1] == "dm")
        {
            crypto.initiateSession(parts[2]); // Ensure crypto session is loaded
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
            crypto.initiateSession(ui.target); // Ensure crypto session is loaded
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
            terminal.printMsg("[!] File not found: " + path);
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
    std::string toSend = crypto.encrypt(ui.target, line);
    net.send((ui.screen == Screen::DM ? "DM " : "MSG ") + ui.target + " " + toSend);
    terminal.printMsg("you: " + line);
}
