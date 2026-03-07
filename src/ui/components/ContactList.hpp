#pragma once
#include <ncurses.h>
#include <string>
#include <vector>
#include <functional>

struct Contact {
    std::string id;
    std::string username;
    bool isOnline;
    bool isTyping;
    int unreadCount;
    std::string statusMessage;
    std::string lastSeen;
};

class ContactList {
private:
    WINDOW* window;
    std::vector<Contact> contacts;
    int selectedIndex;
    int scrollOffset;
    bool showOnlineOnly;
    
public:
    ContactList(WINDOW* win);
    
    void setContacts(const std::vector<Contact>& newContacts);
    void addContact(const Contact& contact);
    void removeContact(const std::string& userId);
    void updateContactStatus(const std::string& userId, bool isOnline);
    void updateTypingStatus(const std::string& userId, bool isTyping);
    void incrementUnread(const std::string& userId);
    void clearUnread(const std::string& userId);
    
    void render();
    void handleKey(int key);
    
    Contact getSelectedContact() const;
    void selectNext();
    void selectPrev();
    
    void setFilter(bool onlineOnly) { showOnlineOnly = onlineOnly; }
    void setWindow(WINDOW* win) { window = win; }
    
    std::function<void(const Contact&)> onContactSelected;
    std::function<void(const Contact&)> onContactDoubleClick;
};