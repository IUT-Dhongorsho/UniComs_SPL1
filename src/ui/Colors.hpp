#pragma once
#include <ncurses.h>

namespace Colors {
    // Color pair indices
    enum ColorPairs {
        HEADER = 1,
        ONLINE = 2,
        OFFLINE = 3,
        OWN_MESSAGE = 4,
        OTHER_MESSAGE = 5,
        TIMESTAMP = 6,
        TYPING = 7,
        UNREAD = 8,
        SELECTED = 9,
        CONTACTS_HEADER = 10,
        HIGHLIGHT = 11,
        INPUT = 12,
        STATUS = 13,
        ERROR = 14,
        SUCCESS = 15,
        WARNING = 16,
        FILE_TRANSFER = 17,
        VOICE_ACTIVE = 18,
        SYSTEM_MESSAGE = 19,
        BORDER = 20,
        TITLE = 21
    };
    
    enum class Theme {
        DARK,
        LIGHT,
        NEON,
        MATRIX
    };
    
    void init();
    void applyTheme(Theme theme);
}