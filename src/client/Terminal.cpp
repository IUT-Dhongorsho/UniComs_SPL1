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
    switch (state.screen)
    {
    case Screen::AUTH:
        return "> ";
    case Screen::MENU:
        return "[" + state.username + "] > ";
    case Screen::DM:
        return "[" + state.username + " \xe2\x86\x92 " + state.target + "] > ";
    case Screen::ROOM:
        return "[" + state.username + " @ " + state.target + "] > ";
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
    std::cout <<"\n\n\n";
    std::cout << "██╗░░░██╗███╗░░██╗██╗░█████╗░░█████╗░███╗░░░███╗░██████╗\n";
    std::cout << "██║░░░██║████╗░██║██║██╔══██╗██╔══██╗████╗░████║██╔════╝\n";
    std::cout << "██║░░░██║██╔██╗██║██║██║░░╚═╝██║░░██║██╔████╔██║╚█████╗░\n";
    std::cout << "██║░░░██║██║╚████║██║██║░░██╗██║░░██║██║╚██╔╝██║░╚═══██╗\n";
    std::cout << "╚██████╔╝██║░╚███║██║╚█████╔╝╚█████╔╝██║░╚═╝░██║██████╔╝\n";
    std::cout << "░╚═════╝░╚═╝░░╚══╝╚═╝░╚════╝░░╚════╝░╚═╝░░░░░╚═╝╚═════╝░\n\n\n";

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
        std::cout << mode << " (Type '/back' to return)\n";

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
    std::cout << "Logged in as " << state.username << ".\n\n";
    std::cout << "  dm     <user>          start a DM\n";
    std::cout << "  join   <room> <pw>     join a room (password protected)\n";
    std::cout << "  create <room> <pw>     create a room (password protected)\n";
    std::cout << "  users                  list users\n";
    std::cout << "  rooms                  list rooms\n";
    std::cout << "  history dm   <user>    DM history\n";
    std::cout << "  history room <room>    room history\n";
    std::cout << "  logout\n\n";
    std::cout << prompt() << std::flush;
}

void Terminal::showChatHelp()
{
    if (state.screen == Screen::ROOM)
    {
        printMsg("  /history           show room chat history");
        printMsg("  /q                 go back to menu");
    }
    else
    { // DM
        printMsg("  /send <filepath>   send a file");
        printMsg("  /call              start a voice call");
        printMsg("  /history           show chat history");
        printMsg("  /accept            accept incoming file or call");
        printMsg("  /reject            reject incoming file or call");
        printMsg("  /endcall           end active call");
        printMsg("  /q                 go back to menu");
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
