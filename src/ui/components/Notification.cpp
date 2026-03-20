#include "Notification.hpp"
#include "../Colors.hpp"

Notification::Notification(WINDOW* parent) : window(parent), timer(0), isVisible(false) {}

Notification::~Notification() {}

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
        default: color = Colors::SYSTEM_MESSAGE; break;
    }
    
    wattron(window, COLOR_PAIR(color) | A_BOLD);
    mvwprintw(window, 2, (width - message.length()) / 2, "%s", message.c_str());
    wattroff(window, COLOR_PAIR(color) | A_BOLD);
}