#include "ContactList.hpp"
#include "../Colors.hpp"
#include <algorithm>

ContactList::ContactList(WINDOW* win) 
    : window(win), selectedIndex(0), scrollOffset(0), showOnlineOnly(false) {}

void ContactList::setContacts(const std::vector<Contact>& newContacts) {
    contacts = newContacts;
    selectedIndex = 0;
    scrollOffset = 0;
}

void ContactList::addContact(const Contact& contact) {
    contacts.push_back(contact);
}

void ContactList::removeContact(const std::string& userId) {
    contacts.erase(
        std::remove_if(contacts.begin(), contacts.end(),
            [&userId](const Contact& c) { return c.id == userId; }),
        contacts.end()
    );
}

void ContactList::updateContactStatus(const std::string& userId, bool isOnline) {
    for (auto& contact : contacts) {
        if (contact.id == userId) {
            contact.isOnline = isOnline;
            break;
        }
    }
}

void ContactList::updateTypingStatus(const std::string& userId, bool isTyping) {
    for (auto& contact : contacts) {
        if (contact.id == userId) {
            contact.isTyping = isTyping;
            break;
        }
    }
}

void ContactList::incrementUnread(const std::string& userId) {
    for (auto& contact : contacts) {
        if (contact.id == userId) {
            contact.unreadCount++;
            break;
        }
    }
}

void ContactList::clearUnread(const std::string& userId) {
    for (auto& contact : contacts) {
        if (contact.id == userId) {
            contact.unreadCount = 0;
            break;
        }
    }
}

void ContactList::render() {
    werase(window);
    box(window, 0, 0);
    
    int height = getmaxy(window);
    int width = getmaxx(window);
    
    // Draw header
    wattron(window, COLOR_PAIR(Colors::CONTACTS_HEADER) | A_BOLD);
    mvwprintw(window, 0, 2, " CONTACTS ");
    if (showOnlineOnly) {
        wattron(window, COLOR_PAIR(Colors::HIGHLIGHT));
        mvwprintw(window, 0, width - 15, "[ONLINE]");
        wattroff(window, COLOR_PAIR(Colors::HIGHLIGHT));
    }
    wattroff(window, COLOR_PAIR(Colors::CONTACTS_HEADER) | A_BOLD);
    
    // Draw filter indicator
    wattron(window, COLOR_PAIR(Colors::TIMESTAMP));
    mvwprintw(window, 1, 2, "F1: %s", showOnlineOnly ? "Show all" : "Online only");
    wattroff(window, COLOR_PAIR(Colors::TIMESTAMP));
    
    int y = 3;
    int visibleCount = 0;
    
    for (size_t i = scrollOffset; i < contacts.size() && y < height - 1; i++) {
        const auto& contact = contacts[i];
        
        if (showOnlineOnly && !contact.isOnline) continue;
        
        // Highlight selected contact
        if (static_cast<int>(i) == selectedIndex) {
            wattron(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
        }
        
        // Online status indicator
        if (contact.isOnline) {
            wattron(window, COLOR_PAIR(Colors::ONLINE));
            mvwprintw(window, y, 2, "●");
            wattroff(window, COLOR_PAIR(Colors::ONLINE));
        } else {
            wattron(window, COLOR_PAIR(Colors::OFFLINE));
            mvwprintw(window, y, 2, "○");
            wattroff(window, COLOR_PAIR(Colors::OFFLINE));
        }
        
        // Username
        mvwprintw(window, y, 4, "%-15s", contact.username.c_str());
        
        // Unread count badge
        if (contact.unreadCount > 0) {
            wattron(window, COLOR_PAIR(Colors::UNREAD) | A_BOLD);
            mvwprintw(window, y, 20, "[%d]", contact.unreadCount);
            wattroff(window, COLOR_PAIR(Colors::UNREAD) | A_BOLD);
        }
        
        // Typing indicator
        if (contact.isTyping) {
            wattron(window, COLOR_PAIR(Colors::TYPING));
            mvwprintw(window, y, 20, "✎");
            wattroff(window, COLOR_PAIR(Colors::TYPING));
        }
        
        // Status message (if any)
        if (!contact.statusMessage.empty() && y < height - 1) {
            wattron(window, COLOR_PAIR(Colors::TIMESTAMP));
            std::string status = contact.statusMessage.substr(0, width - 25);
            mvwprintw(window, y + 1, 4, "%s", status.c_str());
            wattroff(window, COLOR_PAIR(Colors::TIMESTAMP));
        }
        
        if (static_cast<int>(i) == selectedIndex) {
            wattroff(window, COLOR_PAIR(Colors::SELECTED) | A_BOLD);
        }
        
        y += (contact.statusMessage.empty() ? 1 : 2);
        visibleCount++;
    }
    
    // If no contacts visible
    if (visibleCount == 0) {
        wattron(window, COLOR_PAIR(Colors::TIMESTAMP));
        mvwprintw(window, height/2, (width - 20)/2, "No contacts online");
        wattroff(window, COLOR_PAIR(Colors::TIMESTAMP));
    }
    
    wrefresh(window);
}

void ContactList::handleKey(int key) {
    switch(key) {
        case KEY_UP:
            selectPrev();
            break;
        case KEY_DOWN:
            selectNext();
            break;
        case KEY_ENTER:
        case 10:
            if (selectedIndex >= 0 && selectedIndex < static_cast<int>(contacts.size())) {
                if (onContactSelected) {
                    onContactSelected(contacts[selectedIndex]);
                }
            }
            break;
    }
}

void ContactList::selectNext() {
    int oldIndex = selectedIndex;
    do {
        selectedIndex++;
        if (selectedIndex >= static_cast<int>(contacts.size())) {
            selectedIndex = 0;
        }
    } while (showOnlineOnly && !contacts[selectedIndex].isOnline && selectedIndex != oldIndex);
    
    // Adjust scroll
    int height = getmaxy(window);
    if (selectedIndex - scrollOffset >= height - 4) {
        scrollOffset = selectedIndex - (height - 5);
    }
}

void ContactList::selectPrev() {
    int oldIndex = selectedIndex;
    do {
        selectedIndex--;
        if (selectedIndex < 0) {
            selectedIndex = contacts.size() - 1;
        }
    } while (showOnlineOnly && !contacts[selectedIndex].isOnline && selectedIndex != oldIndex);
    
    // Adjust scroll
    if (selectedIndex < scrollOffset) {
        scrollOffset = std::max(0, selectedIndex - 1);
    }
}

Contact ContactList::getSelectedContact() const {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(contacts.size())) {
        return contacts[selectedIndex];
    }
    return Contact{};
}