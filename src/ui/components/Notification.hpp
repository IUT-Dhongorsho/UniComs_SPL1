#pragma once
#include <ncurses.h>
#include <string>

enum class NotifyType {
    INFO,
    SUCCESS,
    ERROR,
    WARNING
};

class Notification {
private:
    WINDOW* window;
    std::string message;
    NotifyType type;
    int timer;
    bool isVisible;
    
public:
    Notification(WINDOW* parent);  // Constructor takes parent window
    ~Notification();
    
    void setWindow(WINDOW* win) { window = win; }  // ADD THIS METHOD
    
    void show(const std::string& msg, NotifyType t = NotifyType::INFO);
    void hide();
    void render();
    void update();
};