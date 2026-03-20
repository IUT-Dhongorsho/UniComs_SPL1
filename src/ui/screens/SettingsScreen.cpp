#include "SettingsScreen.hpp"
#include "../Colors.hpp"

SettingsScreen::SettingsScreen() : window(nullptr), selected(0), isVisible(false) {
    settings = {
        {"Theme", {"Dark", "Light", "Neon", "Matrix"}, 0},
        {"Notifications", {"On", "Off"}, 0},
        {"Sound", {"On", "Off"}, 0}
    };
}

SettingsScreen::~SettingsScreen() {}

void SettingsScreen::setWindow(WINDOW* win) {
    window = win;
}

void SettingsScreen::show() {
    isVisible = true;
}

void SettingsScreen::hide() {
    isVisible = false;
}

void SettingsScreen::render() {
    if (!isVisible || !window) return;
    
    werase(window);
    box(window, 0, 0);
    
    int height, width;
    getmaxyx(window, height, width);
    
    // Title
    wattron(window, COLOR_PAIR(Colors::HEADER) | A_BOLD);
    mvwprintw(window, 1, (width - 12) / 2, "SETTINGS");
    wattroff(window, COLOR_PAIR(Colors::HEADER) | A_BOLD);
    
    // Settings list
    for (size_t i = 0; i < settings.size(); i++) {
        int y = 4 + i * 2;
        
        if (static_cast<int>(i) == selected) {
            wattron(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
            mvwprintw(window, y, 10, "▶");
        }
        
        wattron(window, COLOR_PAIR(Colors::OTHER_MESSAGE));
        mvwprintw(window, y, 12, "%s:", settings[i].name.c_str());
        
        wattron(window, COLOR_PAIR(Colors::SUCCESS));
        mvwprintw(window, y, 30, "%s", 
                  settings[i].options[settings[i].current].c_str());
        
        wattroff(window, COLOR_PAIR(Colors::OTHER_MESSAGE));
        wattroff(window, COLOR_PAIR(Colors::SUCCESS));
        
        if (static_cast<int>(i) == selected) {
            wattroff(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
        }
    }
    
    // Back
    wattron(window, COLOR_PAIR(Colors::WARNING));
    mvwprintw(window, height - 3, 5, "ESC: Back");
    wattroff(window, COLOR_PAIR(Colors::WARNING));
    
    wrefresh(window);
}

bool SettingsScreen::handleKey(int key) {
    switch(key) {
        case KEY_UP:
            if (selected > 0) selected--;
            return true;
        case KEY_DOWN:
            if (selected < static_cast<int>(settings.size()) - 1) selected++;
            return true;
        case KEY_LEFT:
            if (settings[selected].current > 0) {
                settings[selected].current--;
                if (selected == 0 && onThemeChange) {
                    onThemeChange(static_cast<Colors::Theme>(settings[0].current));
                }
            }
            return true;
        case KEY_RIGHT:
            if (settings[selected].current < static_cast<int>(settings[selected].options.size()) - 1) {
                settings[selected].current++;
                if (selected == 0 && onThemeChange) {
                    onThemeChange(static_cast<Colors::Theme>(settings[0].current));
                }
            }
            return true;
        case 27: // ESC
            if (onBack) onBack();
            return true;
    }
    return false;
}