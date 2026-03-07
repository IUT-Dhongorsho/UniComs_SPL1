#include "InputField.hpp"
#include "../Colors.hpp"
#include <cctype>

InputField::InputField(WINDOW* win, const std::string& prompt)
    : window(win), prompt(prompt), cursorPos(0), historyIndex(-1), isActive(true) {}

void InputField::setWindow(WINDOW* win) {
    window = win;
}

void InputField::render() {
    werase(window);
    
    int width = getmaxx(window);
    int y = 0;
    int x = 0;
    
    // Draw prompt
    wattron(window, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
    mvwprintw(window, y, x, "%s", prompt.c_str());
    wattroff(window, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
    
    x += prompt.length();
    
    // Calculate visible part of buffer if it's too long
    std::string visibleBuffer;
    int visibleStart = 0;
    int maxVisible = width - x - 2;
    
    if (static_cast<int>(buffer.length()) > maxVisible) {
        if (cursorPos < maxVisible) {
            visibleBuffer = buffer.substr(0, maxVisible);
        } else if (cursorPos >= static_cast<int>(buffer.length()) - maxVisible) {
            visibleBuffer = buffer.substr(buffer.length() - maxVisible);
            visibleStart = buffer.length() - maxVisible;
        } else {
            visibleBuffer = buffer.substr(cursorPos - maxVisible/2, maxVisible);
            visibleStart = cursorPos - maxVisible/2;
        }
    } else {
        visibleBuffer = buffer;
    }
    
    // Draw buffer
    if (isActive) {
        wattron(window, COLOR_PAIR(Colors::INPUT));
    }
    mvwprintw(window, y, x, "%s", visibleBuffer.c_str());
    
    // Draw cursor
    if (isActive) {
        int cursorScreenX = x + (cursorPos - visibleStart);
        if (cursorScreenX >= x && cursorScreenX < x + maxVisible) {
            wattron(window, A_REVERSE);
            mvwprintw(window, y, cursorScreenX, " ");
            wattroff(window, A_REVERSE);
        }
    }
    
    wattroff(window, COLOR_PAIR(Colors::INPUT));
    
    // Draw mode indicator
    wattron(window, COLOR_PAIR(Colors::TIMESTAMP));
    mvwprintw(window, y, width - 10, "[%s]", isActive ? "INSERT" : "VIEW");
    wattroff(window, COLOR_PAIR(Colors::TIMESTAMP));
    
    wrefresh(window);
}

bool InputField::handleKey(int key) {
    switch(key) {
        case KEY_LEFT:
        case 2:  // Ctrl-B
            moveCursorLeft();
            return true;
            
        case KEY_RIGHT:
        case 6:  // Ctrl-F
            moveCursorRight();
            return true;
            
        case KEY_HOME:
        case 1:  // Ctrl-A
            moveCursorHome();
            return true;
            
        case KEY_END:
        case 5:  // Ctrl-E
            moveCursorEnd();
            return true;
            
        case KEY_BACKSPACE:
        case 127:
            deleteChar();
            return true;
            
        case KEY_DC:
        case 4:  // Ctrl-D
            if (cursorPos < static_cast<int>(buffer.length())) {
                buffer.erase(cursorPos, 1);
            }
            return true;
            
        case KEY_UP:
        case 16:  // Ctrl-P
            historyUp();
            return true;
            
        case KEY_DOWN:
        case 14:  // Ctrl-N
            historyDown();
            return true;
            
        case 9:  // Tab
            if (onTabComplete) {
                onTabComplete(buffer);
            }
            return true;
            
        case 10:  // Enter
        case 13:
            if (!buffer.empty()) {
                addToHistory(buffer);
                if (onSubmit) {
                    onSubmit(buffer);
                }
                clear();
            }
            return true;
            
        default:
            if (isprint(key)) {
                insertChar(static_cast<char>(key));
                return true;
            }
            break;
    }
    return false;
}

void InputField::insertChar(char ch) {
    buffer.insert(cursorPos, 1, ch);
    cursorPos++;
}

void InputField::deleteChar() {
    if (cursorPos > 0) {
        buffer.erase(cursorPos - 1, 1);
        cursorPos--;
    }
}

void InputField::moveCursorLeft() {
    if (cursorPos > 0) cursorPos--;
}

void InputField::moveCursorRight() {
    if (cursorPos < static_cast<int>(buffer.length())) cursorPos++;
}

void InputField::moveCursorHome() {
    cursorPos = 0;
}

void InputField::moveCursorEnd() {
    cursorPos = buffer.length();
}

void InputField::historyUp() {
    if (history.empty()) return;
    if (historyIndex == -1) {
        historyIndex = history.size() - 1;
    } else if (historyIndex > 0) {
        historyIndex--;
    }
    buffer = history[historyIndex];
    cursorPos = buffer.length();
}

void InputField::historyDown() {
    if (history.empty()) return;
    if (historyIndex < static_cast<int>(history.size()) - 1) {
        historyIndex++;
        buffer = history[historyIndex];
    } else if (historyIndex == static_cast<int>(history.size()) - 1) {
        historyIndex = -1;
        buffer.clear();
    }
    cursorPos = buffer.length();
}

void InputField::addToHistory(const std::string& cmd) {
    if (history.empty() || history.back() != cmd) {
        history.push_back(cmd);
        if (history.size() > 100) {  // Limit history size
            history.erase(history.begin());
        }
    }
    historyIndex = -1;
}

void InputField::clear() {
    buffer.clear();
    cursorPos = 0;
}

void InputField::setSuggestions(const std::vector<std::string>& sugg) {
    // TODO: Implement auto-completion dropdown
}