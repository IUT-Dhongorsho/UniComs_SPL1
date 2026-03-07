#include "StatusBar.hpp"
#include "../Colors.hpp"
#include <sstream>
#include <iomanip>

StatusBar::StatusBar(WINDOW* win) 
    : window(win), isConnected(false) {}

void StatusBar::setConnected(bool connected, const std::string& host, int port) {
    isConnected = connected;
    if (connected) {
        std::ostringstream ss;
        ss << " Connected to " << host << ":" << port << " ";
        connectionInfo = ss.str();
    } else {
        connectionInfo = " Disconnected ";
    }
}

void StatusBar::setTyping(const std::string& username) {
    if (username.empty()) {
        typingInfo.clear();
    } else {
        typingInfo = " ✎ " + username + " typing ";
    }
}

void StatusBar::setFileTransfer(const std::string& filename, int progress) {
    if (filename.empty()) {
        fileInfo.clear();
    } else {
        std::ostringstream ss;
        ss << " 📁 " << filename << " " << progress << "% ";
        fileInfo = ss.str();
    }
}

void StatusBar::setVoiceCall(const std::string& username, bool isActive) {
    if (isActive) {
        voiceInfo = " 🎤 Call with " + username + " ";
    } else {
        voiceInfo.clear();
    }
}

void StatusBar::render() {
    werase(window);
    
    int width = getmaxx(window);
    int y = 0;
    
    // Background
    wattron(window, COLOR_PAIR(Colors::STATUS) | A_BOLD);
    mvwhline(window, y, 0, ' ', width);
    
    // Left section - Connection status
    if (!connectionInfo.empty()) {
        if (isConnected) {
            wattron(window, COLOR_PAIR(Colors::SUCCESS));
        } else {
            wattron(window, COLOR_PAIR(Colors::ERROR));
        }
        mvwprintw(window, y, 1, "%s", connectionInfo.c_str());
    }
    
    // Center section - Typing/File/Voice info
    int centerX = width / 2;
    std::string center;
    if (!typingInfo.empty()) {
        center = typingInfo;
        wattron(window, COLOR_PAIR(Colors::TYPING));
    } else if (!fileInfo.empty()) {
        center = fileInfo;
        wattron(window, COLOR_PAIR(Colors::FILE_TRANSFER));
    } else if (!voiceInfo.empty()) {
        center = voiceInfo;
        wattron(window, COLOR_PAIR(Colors::VOICE_ACTIVE));
    }
    
    if (!center.empty()) {
        mvwprintw(window, y, centerX - center.length()/2, "%s", center.c_str());
    }
    
    // Right section - Mode and time
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);
    
    wattron(window, COLOR_PAIR(Colors::TIMESTAMP));
    std::string right = mode + " " + timeStr;
    mvwprintw(window, y, width - right.length() - 2, "%s", right.c_str());
    
    wattroff(window, COLOR_PAIR(Colors::STATUS) | A_BOLD);
    wrefresh(window);
}