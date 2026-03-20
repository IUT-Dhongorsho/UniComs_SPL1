#include "UIManager.hpp"
#include "Animations.hpp"
#include "../utils/utils.h"
#include <chrono>
#include <thread>
#include <cstring>
#include <cctype>
#include <sstream>
#include <iomanip>

UIManager::UIManager() 
    : chatScroll(0), showOnlineOnly(false), 
      currentTheme(Colors::Theme::DARK), m_isRunning(false),
      historyIndex(-1), frameCounter(0), showTypingAnimation(false),
      currentScreen(AppScreen::WELCOME) {  // ADD THIS LINE
    fileTransfer = std::make_unique<FileTransferUI>();
    
    welcomeScreen = new WelcomeScreen();
    loginScreen = new LoginScreen();
    settingsScreen = new SettingsScreen();
    notification = new Notification(nullptr);
}
UIManager::~UIManager() {
    // ADD THESE LINES
    delete welcomeScreen;
    delete loginScreen;
    delete settingsScreen;
    delete notification;
    
    shutdown();
}

void UIManager::initialize() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    timeout(50);
    
    // Initialize colors
    Colors::init();
    Colors::applyTheme(currentTheme);
    
    // Get terminal dimensions
    int height, width;
    getmaxyx(stdscr, height, width);
    
    // Create windows
    title_win = newwin(3, width, 0, 0);
    chat_win = newwin(height - 8, width - 31, 3, 0);
    contacts_win = newwin(height - 8, 30, 3, width - 30);
    input_win = newwin(3, width - 31, height - 5, 0);
    status_win = newwin(1, width, height - 1, 0);
    
    // Enable features
    scrollok(chat_win, TRUE);
    keypad(chat_win, TRUE);
    keypad(input_win, TRUE);
    
    // Create panels
    panels[0] = new_panel(title_win);
    panels[1] = new_panel(chat_win);
    panels[2] = new_panel(contacts_win);
    panels[3] = new_panel(input_win);
    panels[4] = new_panel(status_win);
    
    // Set file transfer window
    fileTransfer->setWindow(newwin(12, 50, height/2 - 6, width/2 - 25));
    
    m_isRunning = true;

    // Set file transfer window
    fileTransfer->setWindow(newwin(12, 50, height/2 - 6, width/2 - 25));
    
    // ADD THESE LINES - Set windows for new screens
    welcomeScreen->setWindow(stdscr);  // Use full screen for welcome
    loginScreen->setWindow(stdscr);    // Use full screen for login
    settingsScreen->setWindow(stdscr); // Use full screen for settings
    notification->setWindow(stdscr);
    
    // ADD THESE LINES - Setup callbacks
    welcomeScreen->onLogin = [this]() { switchScreen(AppScreen::LOGIN); };
    welcomeScreen->onSignup = [this]() { 
        // You can implement signup screen similarly
        // For now, just show notification
        notification->show("Signup not implemented yet", NotifyType::INFO);
    };
    welcomeScreen->onExit = [this]() { 
        m_isRunning = false; 
    };
    
    loginScreen->onLogin = [this](const std::string& user, const std::string& pass) {
        // Here you would call your existing login logic
        // For now, just switch to main screen
        switchScreen(AppScreen::MAIN);
        setCurrentUser(user);
        notification->show("Welcome " + user + "!", NotifyType::SUCCESS);
    };
    loginScreen->onBack = [this]() { switchScreen(AppScreen::WELCOME); };
    
    settingsScreen->onBack = [this]() { switchScreen(AppScreen::MAIN); };
    settingsScreen->onThemeChange = [this](Colors::Theme theme) {
        setTheme(theme);
    };
    
    m_isRunning = true;
}

void UIManager::switchScreen(AppScreen newScreen) {
    // Hide all screens first
    welcomeScreen->hide();
    loginScreen->hide();
    settingsScreen->hide();
    
    // Show the new screen
    currentScreen = newScreen;
    switch(newScreen) {
        case AppScreen::WELCOME:
            welcomeScreen->show();
            break;
        case AppScreen::LOGIN:
            loginScreen->show();
            break;
        case AppScreen::MAIN:
            // Main screen is handled by existing UI
            break;
        case AppScreen::SETTINGS:
            settingsScreen->show();
            break;
    }
}

void UIManager::shutdown() {
    if (m_isRunning) {
        m_isRunning = false;
        for (int i = 0; i < 5; i++) {
            del_panel(panels[i]);
        }
        delwin(title_win);
        delwin(chat_win);
        delwin(contacts_win);
        delwin(input_win);
        delwin(status_win);
        endwin();
    }
}

void UIManager::drawBorderWithTitle(WINDOW* win, const std::string& title) {
    int width = getmaxx(win);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(Colors::HEADER) | A_BOLD);
    mvwprintw(win, 0, 2, "[ %s ]", title.c_str());
    wattroff(win, COLOR_PAIR(Colors::HEADER) | A_BOLD);
}

void UIManager::drawTitle() {
    werase(title_win);
    
    int width = getmaxx(title_win);
    
    // Draw top line
    wattron(title_win, COLOR_PAIR(Colors::HEADER));
    for (int i = 0; i < width; i++) {
        mvwprintw(title_win, 0, i, "=");
    }
    
    // Main title
    std::string title = "== UNICOMS CHAT ";
    if (!currentUser.empty()) {
        title += "| User: " + currentUser + " ";
    }
    title += "==";
    
    wattron(title_win, COLOR_PAIR(Colors::TITLE) | A_BOLD);
    mvwprintw(title_win, 1, (width - title.length()) / 2, "%s", title.c_str());
    wattroff(title_win, COLOR_PAIR(Colors::TITLE) | A_BOLD);
    
    // Bottom line
    for (int i = 0; i < width; i++) {
        mvwprintw(title_win, 2, i, "=");
    }
    
    wattroff(title_win, COLOR_PAIR(Colors::HEADER));
}

void UIManager::drawChatWindow() {
    werase(chat_win);
    
    int height = getmaxy(chat_win);
    int width = getmaxx(chat_win);
    
    // Draw border with title
    std::string chatTitle = " Chat: " + (currentChat.empty() ? "None" : currentChat) + " ";
    drawBorderWithTitle(chat_win, chatTitle);
    
    if (messages.empty()) {
        // Show welcome message
        wattron(chat_win, COLOR_PAIR(Colors::TIMESTAMP) | A_ITALIC);
        mvwprintw(chat_win, height/2, (width - 30)/2, "No messages yet");
        mvwprintw(chat_win, height/2 + 1, (width - 35)/2, "Type /help for commands");
        wattroff(chat_win, COLOR_PAIR(Colors::TIMESTAMP) | A_ITALIC);
    } else {
        // Draw messages from bottom with scrolling
        int visibleLines = height - 3;
        int totalMessages = messages.size();
        
        // Calculate which messages to show based on scroll position
        int startIdx = totalMessages - visibleLines - chatScroll;
        if (startIdx < 0) startIdx = 0;
        if (startIdx > totalMessages - 1) startIdx = totalMessages - 1;
        
        int y = 2; // Start from top
        
        for (int i = startIdx; i < totalMessages && y < height - 2; i++) {
            const auto& msg = messages[i];
            
            // Draw timestamp
            wattron(chat_win, COLOR_PAIR(Colors::TIMESTAMP));
            mvwprintw(chat_win, y, 2, "[%s]", msg.timestamp.substr(11, 5).c_str());
            wattroff(chat_win, COLOR_PAIR(Colors::TIMESTAMP));
            
            // Draw sender
            if (msg.isOwn) {
                wattron(chat_win, COLOR_PAIR(Colors::OWN_MESSAGE) | A_BOLD);
                mvwprintw(chat_win, y, 12, "You:");
                wattroff(chat_win, COLOR_PAIR(Colors::OWN_MESSAGE) | A_BOLD);
            } else {
                wattron(chat_win, COLOR_PAIR(Colors::OTHER_MESSAGE) | A_BOLD);
                mvwprintw(chat_win, y, 12, "%s:", msg.sender.substr(0, 10).c_str());
                wattroff(chat_win, COLOR_PAIR(Colors::OTHER_MESSAGE) | A_BOLD);
            }
            
            // Draw message content (wrap if needed)
            std::string content = msg.content;
            int maxMsgWidth = width - 25;
            if (content.length() > maxMsgWidth) {
                content = content.substr(0, maxMsgWidth - 3) + "...";
            }
            
            if (msg.isOwn) {
                wattron(chat_win, COLOR_PAIR(Colors::OWN_MESSAGE));
            } else {
                wattron(chat_win, COLOR_PAIR(Colors::OTHER_MESSAGE));
            }
            mvwprintw(chat_win, y, 25, "%s", content.c_str());
            
            // Draw status for own messages
            if (msg.isOwn) {
                if (msg.isRead) {
                    mvwprintw(chat_win, y, width - 6, "[read]");
                } else if (msg.isDelivered) {
                    mvwprintw(chat_win, y, width - 6, "[del]");
                } else {
                    mvwprintw(chat_win, y, width - 6, "[sent]");
                }
            }
            
            if (msg.isOwn) {
                wattroff(chat_win, COLOR_PAIR(Colors::OWN_MESSAGE));
            } else {
                wattroff(chat_win, COLOR_PAIR(Colors::OTHER_MESSAGE));
            }
            
            y++;
        }
        
        // Show scroll indicator if needed
        if (chatScroll > 0) {
            wattron(chat_win, COLOR_PAIR(Colors::HIGHLIGHT));
            mvwprintw(chat_win, 1, width - 15, "[↑ More ↑]");
            wattroff(chat_win, COLOR_PAIR(Colors::HIGHLIGHT));
        }
    }
    
    wrefresh(chat_win);
}

void UIManager::drawContacts() {
    werase(contacts_win);
    
    int height = getmaxy(contacts_win);
    int width = getmaxx(contacts_win);
    
    // Draw border with title
    drawBorderWithTitle(contacts_win, " Contacts ");
    
    // Filter button
    if (showOnlineOnly) {
        wattron(contacts_win, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
        mvwprintw(contacts_win, 1, 2, "[X] Online only");
        wattroff(contacts_win, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
    } else {
        wattron(contacts_win, COLOR_PAIR(Colors::TIMESTAMP));
        mvwprintw(contacts_win, 1, 2, "[ ] Online only");
        wattroff(contacts_win, COLOR_PAIR(Colors::TIMESTAMP));
    }
    
    // Draw contacts
    int y = 3;
    int onlineCount = 0;
    
    for (const auto& contact : contacts) {
        if (showOnlineOnly && !contact.isOnline) continue;
        if (y >= height - 2) {
            mvwprintw(contacts_win, y, 2, "...");
            break;
        }
        
        // Online status
        if (contact.isOnline) {
            onlineCount++;
            wattron(contacts_win, COLOR_PAIR(Colors::ONLINE));
            mvwprintw(contacts_win, y, 2, "@");
            wattroff(contacts_win, COLOR_PAIR(Colors::ONLINE));
        } else {
            wattron(contacts_win, COLOR_PAIR(Colors::OFFLINE));
            mvwprintw(contacts_win, y, 2, "o");
            wattroff(contacts_win, COLOR_PAIR(Colors::OFFLINE));
        }
        
        // Contact name with selection highlight
        if (contact.username == currentChat) {
            wattron(contacts_win, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
            mvwprintw(contacts_win, y, 4, "> %-15s", contact.username.c_str());
            wattroff(contacts_win, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
        } else {
            mvwprintw(contacts_win, y, 4, "  %-15s", contact.username.c_str());
        }
        
        // Unread badge
        if (contact.unreadCount > 0) {
            wattron(contacts_win, COLOR_PAIR(Colors::UNREAD) | A_BOLD);
            mvwprintw(contacts_win, y, 22, "(%d)", contact.unreadCount);
            wattroff(contacts_win, COLOR_PAIR(Colors::UNREAD) | A_BOLD);
        }
        
        // Typing indicator
        if (contact.isTyping) {
            wattron(contacts_win, COLOR_PAIR(Colors::TYPING));
            mvwprintw(contacts_win, y, 26, "...");
            wattroff(contacts_win, COLOR_PAIR(Colors::TYPING));
        }
        
        y++;
    }
    
    // Show online count
    wattron(contacts_win, COLOR_PAIR(Colors::TIMESTAMP));
    mvwprintw(contacts_win, height - 1, 2, "Online: %d/%zu", onlineCount, contacts.size());
    wattroff(contacts_win, COLOR_PAIR(Colors::TIMESTAMP));
    
    wrefresh(contacts_win);
}

void UIManager::drawInputBar() {
    werase(input_win);
    
    int width = getmaxx(input_win);
    
    // Draw border
    box(input_win, 0, 0);
    
    // Draw prompt
    wattron(input_win, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
    mvwprintw(input_win, 1, 2, ">>>");
    wattroff(input_win, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
    
    // Draw input buffer
    wattron(input_win, COLOR_PAIR(Colors::INPUT));
    
    std::string displayBuffer = inputBuffer;
    if (displayBuffer.length() > static_cast<size_t>(width - 10)) {
        displayBuffer = "..." + displayBuffer.substr(displayBuffer.length() - (width - 13));
    }
    
    mvwprintw(input_win, 1, 6, "%s", displayBuffer.c_str());
    
    // Draw cursor
    int cursorX = 6 + displayBuffer.length();
    if (cursorX < width - 2) {
        wattron(input_win, A_REVERSE);
        mvwprintw(input_win, 1, cursorX, " ");
        wattroff(input_win, A_REVERSE);
    }
    
    wattroff(input_win, COLOR_PAIR(Colors::INPUT));
    
    // Draw mode indicator
    std::string mode = currentChat.empty() ? "[No chat]" : "[Chat: " + currentChat + "]";
    wattron(input_win, COLOR_PAIR(Colors::TIMESTAMP));
    mvwprintw(input_win, 1, width - mode.length() - 3, "%s", mode.c_str());
    wattroff(input_win, COLOR_PAIR(Colors::TIMESTAMP));
    
    wrefresh(input_win);
}

void UIManager::drawStatusBar() {
    werase(status_win);
    
    int width = getmaxx(status_win);
    
    // Background
    wattron(status_win, COLOR_PAIR(Colors::STATUS) | A_BOLD);
    mvwhline(status_win, 0, 0, ' ', width);
    
    // Connection status
    wattron(status_win, COLOR_PAIR(Colors::SUCCESS));
    mvwprintw(status_win, 0, 2, "CONNECTED");
    
    // Time
    wattron(status_win, COLOR_PAIR(Colors::TIMESTAMP));
    std::string time = getCurrentTime();
    mvwprintw(status_win, 0, width - time.length() - 3, "%s", time.c_str());
    
    // Help shortcuts
    wattron(status_win, COLOR_PAIR(Colors::HIGHLIGHT));
    std::string help = "F1:Filter F2:Clear F3:Files F4:Theme";
    mvwprintw(status_win, 0, (width - help.length()) / 2, "%s", help.c_str());
    
    wattroff(status_win, COLOR_PAIR(Colors::STATUS) | A_BOLD);
    wrefresh(status_win);
}

std::string UIManager::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(localtime(&time), "%H:%M:%S");
    return ss.str();
}

void UIManager::handleResize() {
    int height, width;
    getmaxyx(stdscr, height, width);
    
    wresize(title_win, 3, width);
    wresize(chat_win, height - 8, width - 31);
    wresize(contacts_win, height - 8, 30);
    wresize(input_win, 3, width - 31);
    wresize(status_win, 1, width);
    
    mvwin(chat_win, 3, 0);
    mvwin(contacts_win, 3, width - 30);
    mvwin(input_win, height - 5, 0);
    mvwin(status_win, height - 1, 0);
    
    // Refresh all
    for (int i = 0; i < 5; i++) {
        update_panels();
    }
    doupdate();
}

void UIManager::processInput(int ch) {
    switch(ch) {
        case KEY_RESIZE:
            handleResize();
            break;
            
        case KEY_UP:
            chatScroll++;
            break;
            
        case KEY_DOWN:
            if (chatScroll > 0) {
                chatScroll--;
            }
            break;
            
        case '\t':  // Tab key for navigation
            // Cycle through contacts
            if (!contacts.empty()) {
                static size_t contactIndex = 0;
                contactIndex = (contactIndex + 1) % contacts.size();
                if (onContactSelected && !contacts.empty()) {
                    onContactSelected(contacts[contactIndex].username);
                }
            }
            break;
            
        case KEY_F(1):
            showOnlineOnly = !showOnlineOnly;
            break;
            
        case KEY_F(2):
            inputBuffer.clear();
            break;
            
        case KEY_F(3):
            if (fileTransfer) {
                fileTransfer->show();
            }
            break;
            
        case KEY_F(4):
            {
                int nextTheme = (static_cast<int>(currentTheme) + 1) % 4;
                currentTheme = static_cast<Colors::Theme>(nextTheme);
                Colors::applyTheme(currentTheme);
            }
            break;
            
        case KEY_ENTER:
        case '\n':
        case '\r':
            if (!inputBuffer.empty()) {
                // Store in history
                if (history.empty() || history.back() != inputBuffer) {
                    history.push_back(inputBuffer);
                    if (history.size() > 50) {
                        history.erase(history.begin());
                    }
                }
                historyIndex = -1;
                
                // Send message
                if (onSendMessage) {
                    onSendMessage(inputBuffer);
                }
                inputBuffer.clear();
            }
            break;
            
        case KEY_BACKSPACE:
        case 127:
        case 8:
            if (!inputBuffer.empty()) {
                inputBuffer.pop_back();
            }
            break;
            
        case 21:  // Ctrl+U
            inputBuffer.clear();
            break;
            
        default:
            if (isprint(ch)) {
                inputBuffer += static_cast<char>(ch);
            }
            break;
    }
}

void UIManager::run() {
    while (m_isRunning) {
        int ch = wgetch(input_win);
        
        if (ch != ERR) {
            // Route input to the appropriate screen
            switch(currentScreen) {
                case AppScreen::WELCOME:
                    welcomeScreen->handleKey(ch);
                    break;
                case AppScreen::LOGIN:
                    loginScreen->handleKey(ch);
                    break;
                case AppScreen::SETTINGS:
                    settingsScreen->handleKey(ch);
                    break;
                case AppScreen::MAIN:
                    processInput(ch);  // Your existing input handler
                    break;
            }
        }
        
        // Update animations
        frameCounter++;
        
        // Clear the screen first
        werase(stdscr);
        
        // Draw the appropriate screen
        switch(currentScreen) {
            case AppScreen::WELCOME:
                welcomeScreen->render();
                break;
            case AppScreen::LOGIN:
                loginScreen->render();
                break;
            case AppScreen::SETTINGS:
                settingsScreen->render();
                break;
            case AppScreen::MAIN:
                // Your existing UI drawing code
                drawTitle();
                drawChatWindow();
                drawContacts();
                drawInputBar();
                drawStatusBar();
                break;
        }
        
        // Always draw notifications on top
        notification->update();
        notification->render();
        
        // Draw file transfer if visible
        if (fileTransfer && currentScreen == AppScreen::MAIN) {
            fileTransfer->render();
        }
        
        // Update panels
        update_panels();
        doupdate();
        
        // Small delay for smooth animations
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void UIManager::addMessage(const UIMessage& msg) {
    messages.push_back(msg);
    // Auto-scroll to bottom for new messages
    chatScroll = 0;
}

void UIManager::addMessage(const std::string& chatId, const UIMessage& msg) {
    chatHistory[chatId].push_back(msg);
    if (chatId == currentChat) {
        messages = chatHistory[chatId];
        chatScroll = 0;
    }
}

void UIManager::updateContacts(const std::vector<UIContact>& newContacts) {
    contacts = newContacts;
}

void UIManager::setTypingStatus(const std::string& username, bool isTyping) {
    for (auto& contact : contacts) {
        if (contact.username == username) {
            contact.isTyping = isTyping;
            break;
        }
    }
}

void UIManager::updateMessageStatus(const std::string& msgId, bool delivered, bool read) {
    // Implement if needed
}

void UIManager::showFileProgress(const std::string& filename, int progress, long speed) {
    if (fileTransfer) {
        fileTransfer->show();
    }
}

void UIManager::showFileComplete(const std::string& filename) {
    UIMessage msg;
    msg.id = generateId();
    msg.sender = "System";
    msg.content = "File transfer complete: " + filename;
    msg.timestamp = currentTimestamp();
    msg.isOwn = false;
    msg.type = MessageType::SYSTEM;
    addMessage(msg);
}

void UIManager::showFileError(const std::string& filename, const std::string& error) {
    UIMessage msg;
    msg.id = generateId();
    msg.sender = "System";
    msg.content = "File transfer failed: " + filename + " - " + error;
    msg.timestamp = currentTimestamp();
    msg.isOwn = false;
    msg.type = MessageType::SYSTEM;
    addMessage(msg);
}

void UIManager::showVoiceWave(int amplitude) {
    // Not implemented
}

void UIManager::showVoiceCallStatus(const std::string& contact, bool isCalling) {
    // Not implemented
}

void UIManager::clearChat() {
    messages.clear();
    chatScroll = 0;
}

void UIManager::setTheme(Colors::Theme theme) {
    currentTheme = theme;
    Colors::applyTheme(theme);
}

void UIManager::setCurrentUser(const std::string& username) {
    currentUser = username;
}