#pragma once
#include <string>
#include <vector>
#include <ncurses.h>

enum class MessageType {
    TEXT,
    FILE,
    VOICE,
    SYSTEM
};

class ChatBubble {
private:
    std::string content;
    std::string sender;
    std::string timestamp;
    bool isOwn;
    bool isDelivered;
    bool isRead;
    MessageType type;
    
public:
    ChatBubble(const std::string& content, 
               const std::string& sender,
               const std::string& timestamp,
               bool isOwn,
               bool isDelivered = true,
               bool isRead = false,
               MessageType type = MessageType::TEXT);
    
    void render(WINDOW* win, int y, int x, int maxWidth);
    
private:
    std::vector<std::string> wrapText(const std::string& text, int width);
    void renderTextBubble(WINDOW* win, int y, int x, int maxWidth);
    void renderFileBubble(WINDOW* win, int y, int x);
    void renderVoiceBubble(WINDOW* win, int y, int x);
    std::string getStatusIcon() const;
};