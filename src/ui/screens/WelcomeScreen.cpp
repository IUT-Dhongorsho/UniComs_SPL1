#include "WelcomeScreen.hpp"
#include "../Colors.hpp"
#include "../Animations.hpp"
#include <vector>

WelcomeScreen::WelcomeScreen() : window(nullptr), selectedOption(0), isVisible(false) {}

WelcomeScreen::~WelcomeScreen() {}

void WelcomeScreen::setWindow(WINDOW* win) {
    window = win;
}

void WelcomeScreen::show() {
    isVisible = true;
}

void WelcomeScreen::hide() {
    isVisible = false;
}

void WelcomeScreen::render() {
    if (!isVisible || !window) return;
    
    werase(window);
    
    int height, width;
    getmaxyx(window, height, width);
    
    // ASCII Art Logo
    std::vector<std::string> logo = {
        "╔════════════════════════════════════════════════╗",
        "║   ██╗   ██╗███╗   ██╗██╗ ██████╗ ██████╗ ███╗   ███╗ ███████╗ ║",
        "║   ██║   ██║████╗  ██║██║██╔════╝██╔═══██╗████╗ ████║ ██╔════╝ ║", 
        "║   ██║   ██║██╔██╗ ██║██║██║     ██║   ██║██╔████╔██║ ███████╗ ║",
        "║   ██║   ██║██║╚██╗██║██║██║     ██║   ██║██║╚██╔╝██║ ╚════██║ ║",
        "║   ╚██████╔╝██║ ╚████║██║╚██████╗╚██████╔╝██║ ╚═╝ ██║ ███████║ ║",
        "║    ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝ ╚══════╝ ║",
        "╚════════════════════════════════════════════════╝"
    };
    
    wattron(window, COLOR_PAIR(Colors::TITLE) | A_BOLD);
    int y = 2;
    for (const auto& line : logo) {
        mvwprintw(window, y, (width - line.length()) / 2, "%s", line.c_str());
        y++;
    }
    wattroff(window, COLOR_PAIR(Colors::TITLE) | A_BOLD);
    
    // Menu options
    std::vector<std::string> options = {"LOGIN", "SIGNUP", "EXIT"};
    int startY = 15;
    
    for (size_t i = 0; i < options.size(); i++) {
        if (static_cast<int>(i) == selectedOption) {
            wattron(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
            mvwprintw(window, startY + i * 2, (width - 10) / 2, "▶ %s ◀", options[i].c_str());
            wattroff(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
        } else {
            wattron(window, COLOR_PAIR(Colors::OTHER_MESSAGE));
            mvwprintw(window, startY + i * 2, (width - 8) / 2, "  %s  ", options[i].c_str());
            wattroff(window, COLOR_PAIR(Colors::OTHER_MESSAGE));
        }
    }
    
    // Version
    wattron(window, COLOR_PAIR(Colors::TIMESTAMP));
    mvwprintw(window, height - 3, (width - 15) / 2, "v2.0 - Press ↑/↓, Enter");
    wattroff(window, COLOR_PAIR(Colors::TIMESTAMP));
    
    wrefresh(window);
}

bool WelcomeScreen::handleKey(int key) {
    switch(key) {
        case KEY_UP:
            selectedOption = (selectedOption - 1 + 3) % 3;
            return true;
        case KEY_DOWN:
            selectedOption = (selectedOption + 1) % 3;
            return true;
        case KEY_ENTER:
        case 10:
        case 13:
            if (selectedOption == 0 && onLogin) onLogin();
            else if (selectedOption == 1 && onSignup) onSignup();
            else if (selectedOption == 2 && onExit) onExit();
            return true;
    }
    return false;
}