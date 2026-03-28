#pragma once
#include <string>
#include <mutex>
#include <termios.h>

enum class Screen { AUTH, MENU, DM, ROOM };
enum class AuthStep { CHOOSE_MODE, USERNAME, CONFIRM_SIGNUP, PASSWORD };

struct UIState
{
    Screen   screen       = Screen::AUTH;
    AuthStep authStep     = AuthStep::CHOOSE_MODE;
    bool     signingUp    = false;
    bool     passwordMode = false;
    bool     waiting      = false;
    std::string pendingUsername;
    std::string username;
    std::string target;
    std::string historyPeer;
};

class Terminal
{
public:
    Terminal(UIState &state);
    ~Terminal();

    void enableRawMode();
    void disableRawMode();

    void showAuthScreen();
    void showMenu();
    void showChatHelp();
    
    std::string prompt() const;
    void printMsg(const std::string &msg);
    std::string readLine();

    void setPasswordMode(bool mode) { state.passwordMode = mode; }
    const std::string& getInputBuf() const { return inputBuf; }

private:
    UIState &state;
    struct termios origTermios;
    std::mutex printMtx;
    std::string inputBuf;
};
