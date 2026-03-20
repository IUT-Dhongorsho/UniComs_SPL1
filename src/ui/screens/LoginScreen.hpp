#pragma once
#include <ncurses.h>
#include <functional>
#include <string>

class LoginScreen {
private:
    WINDOW* window;
    std::string username;
    std::string password;
    int cursorField; // 0: username, 1: password
    bool isVisible;
    
public:
    LoginScreen();
    ~LoginScreen();
    
    void setWindow(WINDOW* win);
    void show();
    void hide();
    void render();
    bool handleKey(int key);
    
    std::function<void(const std::string&, const std::string&)> onLogin;
    std::function<void()> onBack;
};