#include "ChatBubble.hpp"
#include "../Colors.hpp"
#include <vector>
#include <sstream>
#include <cstring>

ChatBubble::ChatBubble(const std::string& content, 
                       const std::string& sender,
                       const std::string& timestamp,
                       bool isOwn,
                       bool isDelivered,
                       bool isRead,
                       MessageType type)
    : content(content), sender(sender), timestamp(timestamp),
      isOwn(isOwn), isDelivered(isDelivered), isRead(isRead), type(type) {}

std::vector<std::string> ChatBubble::wrapText(const std::string& text, int width) {
    std::vector<std::string> lines;
    std::string word;
    std::string line;
    std::istringstream iss(text);
    
    while (iss >> word) {
        if (line.length() + word.length() + 1 > static_cast<size_t>(width)) {
            if (!line.empty()) {
                lines.push_back(line);
                line.clear();
            }
            // If word itself is longer than width, split it
            if (word.length() > static_cast<size_t>(width)) {
                for (size_t i = 0; i < word.length(); i += width) {
                    lines.push_back(word.substr(i, width));
                }
                continue;
            }
        }
        if (!line.empty()) line += " ";
        line += word;
    }
    if (!line.empty()) lines.push_back(line);
    
    return lines;
}

void ChatBubble::render(WINDOW* win, int y, int x, int maxWidth) {
    if (type == MessageType::TEXT) {
        renderTextBubble(win, y, x, maxWidth);
    } else if (type == MessageType::FILE) {
        renderFileBubble(win, y, x);
    } else if (type == MessageType::VOICE) {
        renderVoiceBubble(win, y, x);
    }
}

void ChatBubble::renderTextBubble(WINDOW* win, int y, int x, int maxWidth) {
    int bubbleWidth = std::min(maxWidth - 10, 40);
    auto lines = wrapText(content, bubbleWidth - 4);
    
    // Choose color based on own/other
    int colorPair = isOwn ? Colors::OWN_MESSAGE : Colors::OTHER_MESSAGE;
    wattron(win, COLOR_PAIR(colorPair));
    
    if (isOwn) {
        // Right-aligned bubble
        int startX = maxWidth - bubbleWidth - 5;
        
        // Top border
        mvwprintw(win, y, startX, "┌─");
        for (int i = 0; i < bubbleWidth - 4; i++) mvwprintw(win, y, startX + 2 + i, "─");
        mvwprintw(win, y, startX + bubbleWidth - 2, "─┐");
        
        // Message lines
        int lineY = y + 1;
        for (const auto& line : lines) {
            mvwprintw(win, lineY, startX + 2, "%-*s", bubbleWidth - 4, line.c_str());
            lineY++;
        }
        
        // Bottom border with timestamp and status
        mvwprintw(win, lineY, startX, "└─");
        for (int i = 0; i < bubbleWidth - 4; i++) mvwprintw(win, lineY, startX + 2 + i, "─");
        mvwprintw(win, lineY, startX + bubbleWidth - 10, "─┘");
        
        // Timestamp and status
        wattron(win, COLOR_PAIR(Colors::TIMESTAMP));
        mvwprintw(win, lineY, startX + bubbleWidth - 15, "%s", timestamp.c_str());
        mvwprintw(win, lineY, startX + bubbleWidth - 4, "%s", getStatusIcon().c_str());
        wattroff(win, COLOR_PAIR(Colors::TIMESTAMP));
    } else {
        // Left-aligned bubble
        int startX = x + 2;
        
        // Sender name
        wattron(win, A_BOLD);
        mvwprintw(win, y, startX, "%s", sender.c_str());
        wattroff(win, A_BOLD);
        
        // Top border
        mvwprintw(win, y + 1, startX, "┌─");
        for (int i = 0; i < bubbleWidth - 4; i++) mvwprintw(win, y + 1, startX + 2 + i, "─");
        mvwprintw(win, y + 1, startX + bubbleWidth - 2, "─┐");
        
        // Message lines
        int lineY = y + 2;
        for (const auto& line : lines) {
            mvwprintw(win, lineY, startX + 2, "%-*s", bubbleWidth - 4, line.c_str());
            lineY++;
        }
        
        // Bottom border with timestamp
        mvwprintw(win, lineY, startX, "└─");
        for (int i = 0; i < bubbleWidth - 4; i++) mvwprintw(win, lineY, startX + 2 + i, "─");
        mvwprintw(win, lineY, startX + bubbleWidth - 2, "─┘");
        
        // Timestamp
        wattron(win, COLOR_PAIR(Colors::TIMESTAMP));
        mvwprintw(win, lineY, startX + bubbleWidth - 12, "%s", timestamp.c_str());
        wattroff(win, COLOR_PAIR(Colors::TIMESTAMP));
    }
    
    wattroff(win, COLOR_PAIR(colorPair));
}

void ChatBubble::renderFileBubble(WINDOW* win, int y, int x) {
    // TODO: Implement file bubble rendering
    wattron(win, COLOR_PAIR(Colors::FILE_TRANSFER));
    mvwprintw(win, y, x, "📁 %s", content.c_str());
    wattroff(win, COLOR_PAIR(Colors::FILE_TRANSFER));
}

void ChatBubble::renderVoiceBubble(WINDOW* win, int y, int x) {
    // TODO: Implement voice bubble rendering
    wattron(win, COLOR_PAIR(Colors::VOICE_ACTIVE));
    mvwprintw(win, y, x, "🎤 %s", content.c_str());
    wattroff(win, COLOR_PAIR(Colors::VOICE_ACTIVE));
}

std::string ChatBubble::getStatusIcon() const {
    if (isRead) return "✓✓";  // Blue check for read
    if (isDelivered) return "✓✓";  // Gray double check for delivered
    return "✓";  // Single check for sent
}