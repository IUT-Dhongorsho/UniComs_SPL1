#pragma once
#include <ncurses.h>
#include <functional>

class WelcomeScreen {
private:
    WINDOW* window;
    int selectedOption;
    bool isVisible;
    
public:
    WelcomeScreen();
    ~WelcomeScreen();
    
    void setWindow(WINDOW* win);
    void show();
    void hide();
    void render();
    bool handleKey(int key);
    
    std::function<void()> onLogin;
    std::function<void()> onSignup;
    std::function<void()> onExit;
};