#pragma once
#include <ncurses.h>
#include <string>
#include <functional>
#include <vector>

class InputField {
private:
    WINDOW* window;
    std::string buffer;
    std::string prompt;
    int cursorPos;
    std::vector<std::string> history;
    int historyIndex;
    bool isActive;
    
public:
    InputField(WINDOW* win, const std::string& prompt = "> ");
    
    void setWindow(WINDOW* win);
    void setPrompt(const std::string& newPrompt) { prompt = newPrompt; }
    
    void render();
    bool handleKey(int key);
    
    std::string getBuffer() const { return buffer; }
    void setBuffer(const std::string& text) { buffer = text; cursorPos = buffer.length(); }
    void clear();
    
    // Callbacks
    std::function<void(const std::string&)> onSubmit;
    std::function<void(const std::string&)> onTabComplete;
    
    // Auto-completion suggestions
    void setSuggestions(const std::vector<std::string>& sugg);
    
private:
    void insertChar(char ch);
    void deleteChar();
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorHome();
    void moveCursorEnd();
    void historyUp();
    void historyDown();
    void addToHistory(const std::string& cmd);
};