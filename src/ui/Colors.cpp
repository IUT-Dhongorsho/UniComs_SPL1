#include "Colors.hpp"
#include <cstdlib>

namespace Colors {
    
void init() {
    start_color();
    
    // Define standard colors only (no custom colors)
    applyTheme(Theme::DARK); // Default
}

void applyTheme(Theme theme) {
    switch(theme) {
        case Theme::DARK:
            // Modern dark theme
            init_pair(HEADER, COLOR_CYAN, COLOR_BLACK);
            init_pair(ONLINE, COLOR_GREEN, COLOR_BLACK);
            init_pair(OFFLINE, COLOR_WHITE, COLOR_BLACK);
            init_pair(OWN_MESSAGE, COLOR_GREEN, COLOR_BLACK);
            init_pair(OTHER_MESSAGE, COLOR_WHITE, COLOR_BLACK);
            init_pair(TIMESTAMP, COLOR_YELLOW, COLOR_BLACK);
            init_pair(TYPING, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(UNREAD, COLOR_RED, COLOR_BLACK);
            init_pair(SELECTED, COLOR_BLUE, COLOR_WHITE);
            init_pair(CONTACTS_HEADER, COLOR_YELLOW, COLOR_BLACK);
            init_pair(HIGHLIGHT, COLOR_CYAN, COLOR_BLACK);
            init_pair(INPUT, COLOR_WHITE, COLOR_BLACK);
            init_pair(STATUS, COLOR_BLACK, COLOR_WHITE);
            init_pair(ERROR, COLOR_RED, COLOR_BLACK);
            init_pair(SUCCESS, COLOR_GREEN, COLOR_BLACK);
            init_pair(WARNING, COLOR_YELLOW, COLOR_BLACK);
            init_pair(FILE_TRANSFER, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(VOICE_ACTIVE, COLOR_RED, COLOR_BLACK);
            init_pair(SYSTEM_MESSAGE, COLOR_CYAN, COLOR_BLACK);
            init_pair(BORDER, COLOR_BLUE, COLOR_BLACK);
            init_pair(TITLE, COLOR_CYAN, COLOR_BLACK);
            break;
            
        case Theme::LIGHT:
            // Clean light theme
            init_pair(HEADER, COLOR_BLUE, COLOR_WHITE);
            init_pair(ONLINE, COLOR_GREEN, COLOR_WHITE);
            init_pair(OFFLINE, COLOR_BLACK, COLOR_WHITE);
            init_pair(OWN_MESSAGE, COLOR_GREEN, COLOR_WHITE);
            init_pair(OTHER_MESSAGE, COLOR_BLACK, COLOR_WHITE);
            init_pair(TIMESTAMP, COLOR_MAGENTA, COLOR_WHITE);
            init_pair(TYPING, COLOR_RED, COLOR_WHITE);
            init_pair(UNREAD, COLOR_RED, COLOR_WHITE);
            init_pair(SELECTED, COLOR_WHITE, COLOR_BLUE);
            init_pair(CONTACTS_HEADER, COLOR_BLUE, COLOR_WHITE);
            init_pair(HIGHLIGHT, COLOR_CYAN, COLOR_WHITE);
            init_pair(INPUT, COLOR_BLACK, COLOR_WHITE);
            init_pair(STATUS, COLOR_WHITE, COLOR_BLACK);
            init_pair(ERROR, COLOR_RED, COLOR_WHITE);
            init_pair(SUCCESS, COLOR_GREEN, COLOR_WHITE);
            init_pair(WARNING, COLOR_YELLOW, COLOR_WHITE);
            init_pair(FILE_TRANSFER, COLOR_MAGENTA, COLOR_WHITE);
            init_pair(VOICE_ACTIVE, COLOR_RED, COLOR_WHITE);
            init_pair(SYSTEM_MESSAGE, COLOR_BLUE, COLOR_WHITE);
            init_pair(BORDER, COLOR_BLACK, COLOR_WHITE);
            init_pair(TITLE, COLOR_BLUE, COLOR_WHITE);
            break;
            
        case Theme::NEON:
            // Cyberpunk neon theme
            init_pair(HEADER, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(ONLINE, COLOR_GREEN, COLOR_BLACK);
            init_pair(OFFLINE, COLOR_CYAN, COLOR_BLACK);
            init_pair(OWN_MESSAGE, COLOR_GREEN, COLOR_BLACK);
            init_pair(OTHER_MESSAGE, COLOR_CYAN, COLOR_BLACK);
            init_pair(TIMESTAMP, COLOR_YELLOW, COLOR_BLACK);
            init_pair(TYPING, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(UNREAD, COLOR_RED, COLOR_BLACK);
            init_pair(SELECTED, COLOR_WHITE, COLOR_MAGENTA);
            init_pair(CONTACTS_HEADER, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(HIGHLIGHT, COLOR_GREEN, COLOR_BLACK);
            init_pair(INPUT, COLOR_WHITE, COLOR_BLACK);
            init_pair(STATUS, COLOR_BLACK, COLOR_MAGENTA);
            init_pair(ERROR, COLOR_RED, COLOR_BLACK);
            init_pair(SUCCESS, COLOR_GREEN, COLOR_BLACK);
            init_pair(WARNING, COLOR_YELLOW, COLOR_BLACK);
            init_pair(FILE_TRANSFER, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(VOICE_ACTIVE, COLOR_RED, COLOR_BLACK);
            init_pair(SYSTEM_MESSAGE, COLOR_CYAN, COLOR_BLACK);
            init_pair(BORDER, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(TITLE, COLOR_MAGENTA, COLOR_BLACK);
            break;
            
        case Theme::MATRIX:
            // Matrix green theme
            init_pair(HEADER, COLOR_GREEN, COLOR_BLACK);
            init_pair(ONLINE, COLOR_GREEN, COLOR_BLACK);
            init_pair(OFFLINE, COLOR_GREEN, COLOR_BLACK);
            init_pair(OWN_MESSAGE, COLOR_GREEN, COLOR_BLACK);
            init_pair(OTHER_MESSAGE, COLOR_GREEN, COLOR_BLACK);
            init_pair(TIMESTAMP, COLOR_GREEN, COLOR_BLACK);
            init_pair(TYPING, COLOR_GREEN, COLOR_BLACK);
            init_pair(UNREAD, COLOR_GREEN, COLOR_BLACK);
            init_pair(SELECTED, COLOR_BLACK, COLOR_GREEN);
            init_pair(CONTACTS_HEADER, COLOR_GREEN, COLOR_BLACK);
            init_pair(HIGHLIGHT, COLOR_GREEN, COLOR_BLACK);
            init_pair(INPUT, COLOR_GREEN, COLOR_BLACK);
            init_pair(STATUS, COLOR_BLACK, COLOR_GREEN);
            init_pair(ERROR, COLOR_GREEN, COLOR_BLACK);
            init_pair(SUCCESS, COLOR_GREEN, COLOR_BLACK);
            init_pair(WARNING, COLOR_GREEN, COLOR_BLACK);
            init_pair(FILE_TRANSFER, COLOR_GREEN, COLOR_BLACK);
            init_pair(VOICE_ACTIVE, COLOR_GREEN, COLOR_BLACK);
            init_pair(SYSTEM_MESSAGE, COLOR_GREEN, COLOR_BLACK);
            init_pair(BORDER, COLOR_GREEN, COLOR_BLACK);
            init_pair(TITLE, COLOR_GREEN, COLOR_BLACK);
            break;
    }
}

} // namespace Colors