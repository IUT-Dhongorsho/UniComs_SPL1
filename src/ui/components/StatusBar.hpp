#pragma once
#include <ncurses.h>
#include <string>

class StatusBar {
private:
    WINDOW* window;
    std::string leftText;
    std::string centerText;
    std::string rightText;
    std::string mode;
    
public:
    StatusBar(WINDOW* win);
    
    void setWindow(WINDOW* win) { window = win; }
    
    // Update status
    void setLeft(const std::string& text) { leftText = text; }
    void setCenter(const std::string& text) { centerText = text; }
    void setRight(const std::string& text) { rightText = text; }
    void setMode(const std::string& newMode) { mode = newMode; }
    
    // Connection status
    void setConnected(bool connected, const std::string& host = "", int port = 0);
    void setTyping(const std::string& username);
    void setFileTransfer(const std::string& filename, int progress);
    void setVoiceCall(const std::string& username, bool isActive);
    
    void render();
    
private:
    bool isConnected;
    std::string connectionInfo;
    std::string typingInfo;
    std::string fileInfo;
    std::string voiceInfo;
};