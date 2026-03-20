#include "LoginScreen.hpp"
#include "../Colors.hpp"
#include "../components/InputField.hpp"

LoginScreen::LoginScreen() : window(nullptr), cursorField(0), isVisible(false) {}

LoginScreen::~LoginScreen() {}

void LoginScreen::setWindow(WINDOW* win) {
    window = win;
}

void LoginScreen::show() {
    isVisible = true;
    username.clear();
    password.clear();
    cursorField = 0;
}

void LoginScreen::hide() {
    isVisible = false;
}

void LoginScreen::render() {
    if (!isVisible || !window) return;
    
    werase(window);
    box(window, 0, 0);
    
    int height, width;
    getmaxyx(window, height, width);
    
    // Title
    wattron(window, COLOR_PAIR(Colors::HEADER) | A_BOLD);
    mvwprintw(window, 2, (width - 12) / 2, "LOGIN");
    wattroff(window, COLOR_PAIR(Colors::HEADER) | A_BOLD);
    
    // Username
    wattron(window, COLOR_PAIR(Colors::HIGHLIGHT));
    mvwprintw(window, 6, 10, "Username: %s", username.c_str());
    if (cursorField == 0) {
        wattron(window, A_REVERSE);
        mvwprintw(window, 6, 20 + username.length(), " ");
        wattroff(window, A_REVERSE);
    }
    
    // Password
    std::string hiddenPass(password.length(), '*');
    mvwprintw(window, 8, 10, "Password: %s", hiddenPass.c_str());
    if (cursorField == 1) {
        wattron(window, A_REVERSE);
        mvwprintw(window, 8, 20 + password.length(), " ");
        wattroff(window, A_REVERSE);
    }
    wattroff(window, COLOR_PAIR(Colors::HIGHLIGHT));
    
    // Login button
    if (cursorField == 2) {
        wattron(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
        mvwprintw(window, 11, (width - 12) / 2, "[ LOGIN ]");
        wattroff(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
    } else {
        wattron(window, COLOR_PAIR(Colors::SUCCESS));
        mvwprintw(window, 11, (width - 8) / 2, "LOGIN");
        wattroff(window, COLOR_PAIR(Colors::SUCCESS));
    }
    
    // Back
    wattron(window, COLOR_PAIR(Colors::WARNING));
    mvwprintw(window, height - 3, 5, "ESC: Back");
    wattroff(window, COLOR_PAIR(Colors::WARNING));
    
    wrefresh(window);
}

bool LoginScreen::handleKey(int key) {
    if (key == '\t') {
        cursorField = (cursorField + 1) % 3;
        return true;
    }
    
    if (key == KEY_UP) {
        cursorField = (cursorField - 1 + 3) % 3;
        return true;
    }
    
    if (key == KEY_DOWN) {
        cursorField = (cursorField + 1) % 3;
        return true;
    }
    
    if (key == KEY_BACKSPACE || key == 127) {
        if (cursorField == 0 && !username.empty()) username.pop_back();
        if (cursorField == 1 && !password.empty()) password.pop_back();
        return true;
    }
    
    if (key == KEY_ENTER || key == 10 || key == 13) {
        if (cursorField == 2 && onLogin && !username.empty() && !password.empty()) {
            onLogin(username, password);
        }
        return true;
    }
    
    if (key == 27) { // ESC
        if (onBack) onBack();
        return true;
    }
    
    if (isprint(key)) {
        if (cursorField == 0 && username.length() < 30) {
            username += static_cast<char>(key);
        } else if (cursorField == 1 && password.length() < 50) {
            password += static_cast<char>(key);
        }
        return true;
    }
    
    return false;
}