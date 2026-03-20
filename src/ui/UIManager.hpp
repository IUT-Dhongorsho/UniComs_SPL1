#pragma once
#include <ncurses.h>
#include <panel.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <map>
#include "Colors.hpp"
#include "components/ChatBubble.hpp"
#include "components/ContactList.hpp"
#include "components/InputField.hpp"
#include "components/StatusBar.hpp"
#include "components/FileTransferUI.hpp"
#include "screens/WelcomeScreen.hpp"
#include "screens/LoginScreen.hpp"
#include "screens/SettingsScreen.hpp"
#include "components/Notification.hpp"

// Message structure for UI
struct UIMessage {
    std::string id;
    std::string sender;
    std::string content;
    std::string timestamp;
    bool isOwn;
    bool isDelivered;
    bool isRead;
    MessageType type;
};

// Contact structure
struct UIContact {
    std::string id;
    std::string username;
    std::string status;
    bool isOnline;
    bool isTyping;
    int unreadCount;
    std::string lastSeen;
};

// Callback types
using MessageCallback = std::function<void(const std::string&)>;
using CommandCallback = std::function<void(const std::string&)>;
using FileCallback = std::function<void(const std::string&, const std::string&)>;
using ContactCallback = std::function<void(const std::string&)>;

class UIManager {
private:
    // Windows
    WINDOW* main_win;
    WINDOW* chat_win;
    WINDOW* input_win;
    WINDOW* contacts_win;
    WINDOW* status_win;
    WINDOW* title_win;
    PANEL* panels[5];
    
    // Data
    std::vector<UIMessage> messages;
    std::map<std::string, std::vector<UIMessage>> chatHistory;
    std::vector<UIContact> contacts;
    std::string currentUser;
    std::string currentChat;
    std::atomic<bool> m_isRunning;
    
    // UI State
    int chatScroll;
    bool showOnlineOnly;
    Colors::Theme currentTheme;
    std::string inputBuffer;
    std::vector<std::string> history;
    int historyIndex;
    
    // Callbacks
    MessageCallback onSendMessage;
    CommandCallback onCommand;
    FileCallback onFileTransfer;
    ContactCallback onContactSelected;
    
    // File transfer
    std::unique_ptr<FileTransferUI> fileTransfer;
    
    // Animation state
    int frameCounter;
    bool showTypingAnimation;
    
    // Helper methods
    void drawTitle();
    void drawChatWindow();
    void drawContacts();
    void drawInputBar();
    void drawStatusBar();
    void handleResize();
    void processInput(int ch);
    void drawBorderWithTitle(WINDOW* win, const std::string& title);
    std::string getCurrentTime();

    enum class AppScreen {
        WELCOME,
        LOGIN,
        MAIN,
        SETTINGS
    };
    
    AppScreen currentScreen;
    WelcomeScreen* welcomeScreen;
    LoginScreen* loginScreen;
    SettingsScreen* settingsScreen;
    Notification* notification;
    
    void switchScreen(AppScreen newScreen);
    
public:
    UIManager();
    ~UIManager();
    
    // Initialization
    void initialize();
    void shutdown();
    
    // Callback setters
    void setMessageCallback(MessageCallback cb) { onSendMessage = cb; }
    void setCommandCallback(CommandCallback cb) { onCommand = cb; }
    void setFileCallback(FileCallback cb) { onFileTransfer = cb; }
    void setContactCallback(ContactCallback cb) { onContactSelected = cb; }
    
    // Data update methods
    void addMessage(const UIMessage& msg);
    void addMessage(const std::string& chatId, const UIMessage& msg);
    void updateContacts(const std::vector<UIContact>& newContacts);
    void setCurrentUser(const std::string& username) { currentUser = username; }
    void setCurrentChat(const std::string& chat) { 
        currentChat = chat; 
        // Load chat history when switching chats
        if (chatHistory.find(chat) != chatHistory.end()) {
            messages = chatHistory[chat];
        } else {
            messages.clear();
        }
        chatScroll = 0;
    }
    void setTypingStatus(const std::string& username, bool isTyping);
    void updateMessageStatus(const std::string& msgId, bool delivered, bool read);
    
    // File transfer
    void showFileProgress(const std::string& filename, int progress, long speed);
    void showFileComplete(const std::string& filename);
    void showFileError(const std::string& filename, const std::string& error);
    
    // Voice chat
    void showVoiceWave(int amplitude);
    void showVoiceCallStatus(const std::string& contact, bool isCalling);
    
    // Main loop
    void run();
    bool isRunning() const { return m_isRunning; }
    
    // Utility
    void clearChat();
    void setTheme(Colors::Theme theme);
};