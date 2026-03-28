#include "Terminal.h"
#include <iostream>
#include <unistd.h>
#include <termios.h>

Terminal::Terminal(UIState &state) : state(state) {}

Terminal::~Terminal()
{
    disableRawMode();
}

void Terminal::enableRawMode()
{
    tcgetattr(STDIN_FILENO, &origTermios);
    struct termios raw = origTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void Terminal::disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
}

std::string Terminal::prompt() const
{
    std::string self = BOLD GREEN + state.username + RESET;
    switch (state.screen)
    {
    case Screen::AUTH:
        return "> ";
    case Screen::MENU:
        return "[" + self + "] > ";
    case Screen::DM:
    {
        std::string target = BOLD YELLOW + state.target + RESET;
        return "[" + self + " \xe2\x86\x92 " + target + "] > ";
    }
    case Screen::ROOM:
    {
        std::string target = BOLD YELLOW + state.target + RESET;
        return "[" + self + " @ " + target + "] > ";
    }
    }
    return "> ";
}

void Terminal::printMsg(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(printMtx);
    std::cout << "\r\033[K" << msg << "\n"
              << prompt() << inputBuf << std::flush;
}

void Terminal::showAuthScreen()
{
    std::cout << "\033[2J\033[H";
    std::cout << CYAN << "\n\n\n";
    std::cout << "██╗░░░██╗███╗░░██╗██╗░█████╗░░█████╗░███╗░░░███╗░██████╗\n";
    std::cout << "██║░░░██║████╗░██║██║██╔══██╗██╔══██╗████╗░████║██╔════╝\n";
    std::cout << "██║░░░██║██╔██╗██║██║██║░░╚═╝██║░░██║██╔████╔██║╚█████╗░\n";
    std::cout << "██║░░░██║██║╚████║██║██║░░██╗██║░░██╗██║╚██╔╝██║░╚═══██╗\n";
    std::cout << "╚██████╔╝██║░╚███║██║╚█████╔╝╚█████╔╝██║░╚═╝░██║██████╔╝\n";
    std::cout << "░╚═════╝░╚═╝░░╚══╝╚═╝░╚════╝░░╚════╝░╚═╝░░░░░╚═╝╚═════╝░\n\n\n" << RESET;

    if (state.authStep == AuthStep::CHOOSE_MODE)
    {
        std::cout << "Select an option:\n";
        std::cout << "  1. Login\n";
        std::cout << "  2. Signup\n";
        std::cout << "\nChoice: " << std::flush;
    }
    else
    {
        std::string mode = state.signingUp ? "[SIGNUP]" : "[LOGIN]";
        // Updated instruction to match the strict /back requirement
        std::cout << BOLD MAGENTA << mode << RESET << " (Type '/back' to return)\n";

        if (state.authStep == AuthStep::USERNAME)
        {
            std::cout << "Enter username: " << std::flush;
        }
        else if (state.authStep == AuthStep::PASSWORD)
        {
            std::cout << (state.signingUp ? "Choose password: " : "Enter password: ") << std::flush;
        }
    }
}

void Terminal::showMenu()
{
    std::cout << "\033[2J\033[H";
    std::cout << "Logged in as " << BOLD GREEN << state.username << RESET << ".\n\n";
    std::cout << "  " << CYAN << "dm" << RESET << "     <user>          start a DM\n";
    std::cout << "  " << CYAN << "join" << RESET << "   <room> <pw>     join a room (password protected)\n";
    std::cout << "  " << CYAN << "create" << RESET << " <room> <pw>     create a room (password protected)\n";
    std::cout << "  " << CYAN << "users" << RESET << "                  list users\n";
    std::cout << "  " << CYAN << "rooms" << RESET << "                  list rooms\n";
    std::cout << "  " << CYAN << "history" << RESET << " dm   <user>    DM history\n";
    std::cout << "  " << CYAN << "history" << RESET << " room <room>    room history\n";
    std::cout << "  " << CYAN << "logout" << RESET << "\n\n";
    std::cout << prompt() << std::flush;
}

void Terminal::showChatHelp()
{
    if (state.screen == Screen::ROOM)
    {
        printMsg("  " + std::string(CYAN) + "/history" + RESET + "           show room chat history");
        printMsg("  " + std::string(CYAN) + "/q" + RESET + "                 go back to menu");
    }
    else
    { // DM
        printMsg("  " + std::string(CYAN) + "/send" + RESET + " <filepath>   send a file");
        printMsg("  " + std::string(CYAN) + "/call" + RESET + "              start a voice call");
        printMsg("  " + std::string(CYAN) + "/history" + RESET + "           show chat history");
        printMsg("  " + std::string(CYAN) + "/accept" + RESET + "            accept incoming file or call");
        printMsg("  " + std::string(CYAN) + "/reject" + RESET + "            reject incoming file or call");
        printMsg("  " + std::string(CYAN) + "/endcall" + RESET + "           end active call");
        printMsg("  " + std::string(CYAN) + "/q" + RESET + "                 go back to menu");
    }
}

std::string Terminal::readLine()
{
    inputBuf.clear();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (c == '\n' || c == '\r')
        {
            std::cout << "\n"
                      << std::flush;
            std::string result = inputBuf;
            inputBuf.clear();
            return result;
        }
        if (c == 127 || c == '\b')
        {
            if (!inputBuf.empty())
            {
                inputBuf.pop_back();
                if (!state.passwordMode)
                    std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        if (c == 3)
        {
            disableRawMode();
            std::cout << "\n";
            exit(0);
        }
        inputBuf += c;
        if (!state.passwordMode)
            std::cout << c << std::flush;
    }
    return "";
}
