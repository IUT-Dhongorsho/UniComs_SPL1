#include "Notification.hpp"
#include "../Colors.hpp"

Notification::Notification(WINDOW* parent) 
    : window(parent), timer(0), isVisible(false) {}

Notification::~Notification() {}

void Notification::setWindow(WINDOW* win) {
    window = win;
}

void Notification::show(const std::string& msg, NotifyType t) {
    message = msg;
    type = t;
    timer = 50; // ~2.5 seconds at 20fps
    isVisible = true;
}

void Notification::hide() {
    isVisible = false;
}

void Notification::update() {
    if (timer > 0) {
        timer--;
        if (timer == 0) isVisible = false;
    }
}

void Notification::render() {
    if (!isVisible || !window) return;
    
    int height, width;
    getmaxyx(window, height, width);
    
    int color;
    switch(type) {
        case NotifyType::SUCCESS: color = Colors::SUCCESS; break;
        case NotifyType::ERROR: color = Colors::ERROR; break;
        case NotifyType::WARNING: color = Colors::WARNING; break;
        default: color = Colors::SYSTEM_MESSAGE; break;
    }
    
    // Draw notification with ASCII characters
    int msgWidth = message.length() + 4;
    int startX = (width - msgWidth) / 2;
    int startY = 2;
    
    wattron(window, COLOR_PAIR(color) | A_BOLD);
    
    // Use simple ASCII characters instead of Unicode
    mvwprintw(window, startY, startX, "+%s+", std::string(msgWidth - 2, '-').c_str());
    mvwprintw(window, startY + 1, startX, "| %s |", message.c_str());
    mvwprintw(window, startY + 2, startX, "+%s+", std::string(msgWidth - 2, '-').c_str());
    
    wattroff(window, COLOR_PAIR(color) | A_BOLD);
    
    wrefresh(window);
}