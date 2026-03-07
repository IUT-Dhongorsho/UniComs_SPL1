#include "Animations.hpp"
#include "Colors.hpp"
#include <cstring>
#include <cstdlib>
#include <ctime>

void Animations::loadingSpinner(WINDOW* win, int y, int x, const std::string& message, int duration_ms) {
    const char spinner[] = "|/-\\";
    int num_frames = 4;
    int iterations = duration_ms / 100; // Change every 100ms
    
    wattron(win, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
    for(int i = 0; i < iterations; i++) {
        mvwprintw(win, y, x, "%c %s", spinner[i % num_frames], message.c_str());
        wrefresh(win);
        sleep_ms(100);
    }
    wattroff(win, COLOR_PAIR(Colors::HIGHLIGHT) | A_BOLD);
}

void Animations::typingIndicator(WINDOW* win, int y, int x, const std::string& username, int cycles) {
    const char* dots[] = {".  ", ".. ", "..."};
    
    wattron(win, COLOR_PAIR(Colors::TYPING) | A_ITALIC);
    for(int i = 0; i < cycles; i++) {
        mvwprintw(win, y, x, "✎ %s is typing%s", username.c_str(), dots[i % 3]);
        wrefresh(win);
        sleep_ms(300);
    }
    // Clear the indicator
    mvwprintw(win, y, x, "%-*s", 30, " ");
    wattroff(win, COLOR_PAIR(Colors::TYPING) | A_ITALIC);
}

void Animations::progressBar(WINDOW* win, int y, int x, int width, int progress, const std::string& label) {
    int filled = (progress * width) / 100;
    
    // Draw label
    if (!label.empty()) {
        wattron(win, A_BOLD);
        mvwprintw(win, y, x, "%s: ", label.c_str());
        wattroff(win, A_BOLD);
        x += label.length() + 2;
    }
    
    // Draw progress bar
    wattron(win, COLOR_PAIR(Colors::FILE_TRANSFER));
    mvwprintw(win, y, x, "[");
    
    for(int i = 0; i < width; i++) {
        if(i < filled) {
            // Gradient effect based on progress
            if(progress < 30) wattron(win, COLOR_PAIR(Colors::WARNING));
            else if(progress < 70) wattron(win, COLOR_PAIR(Colors::HIGHLIGHT));
            else wattron(win, COLOR_PAIR(Colors::SUCCESS));
            
            mvwprintw(win, y, x + i + 1, "█");
        } else {
            wattron(win, COLOR_PAIR(Colors::OFFLINE));
            mvwprintw(win, y, x + i + 1, "░");
        }
    }
    
    mvwprintw(win, y, x + width + 1, "] %d%%", progress);
    wrefresh(win);
    wattroff(win, COLOR_PAIR(Colors::FILE_TRANSFER));
}

void Animations::waveAnimation(WINDOW* win, int y, int x, int width, int amplitude) {
    const char waves[] = "▁▂▃▄▅▆▇█▇▆▅▄▃▂▁";
    int wave_len = strlen(waves);
    
    wattron(win, COLOR_PAIR(Colors::VOICE_ACTIVE));
    for(int i = 0; i < width * 2; i++) {
        for(int j = 0; j < width; j++) {
            int wave_idx = (i + j * 2) % wave_len;
            mvwprintw(win, y, x + j, "%c", waves[wave_idx]);
        }
        wrefresh(win);
        sleep_ms(50);
    }
    wattroff(win, COLOR_PAIR(Colors::VOICE_ACTIVE));
}

void Animations::pulse(WINDOW* win, int y, int x, const std::string& text, int color_pair, int pulses) {
    for(int i = 0; i < pulses; i++) {
        // Fade in
        for(int j = 0; j <= 3; j++) {
            wattron(win, COLOR_PAIR(color_pair) | (j == 3 ? A_BOLD : 0));
            mvwprintw(win, y, x, "%s", text.c_str());
            wrefresh(win);
            sleep_ms(50);
        }
        // Fade out
        for(int j = 3; j >= 0; j--) {
            wattron(win, COLOR_PAIR(color_pair) | (j == 3 ? A_BOLD : 0));
            mvwprintw(win, y, x, "%s", text.c_str());
            wrefresh(win);
            sleep_ms(50);
        }
    }
}

void Animations::scrollText(WINDOW* win, int y, int x, const std::string& text, int width, int speed_ms) {
    if (text.length() <= static_cast<size_t>(width)) {
        mvwprintw(win, y, x, "%s", text.c_str());
        wrefresh(win);
        return;
    }
    
    std::string spaced = text + "   ";
    for(size_t i = 0; i < spaced.length(); i++) {
        std::string visible = spaced.substr(i, width);
        mvwprintw(win, y, x, "%s", visible.c_str());
        wrefresh(win);
        sleep_ms(speed_ms);
    }
}

void Animations::fadeIn(WINDOW* win, int y, int x, const std::string& text, int color_pair) {
    for(int i = 0; i <= 100; i += 10) {
        wattron(win, COLOR_PAIR(color_pair) | (i > 50 ? A_BOLD : 0));
        mvwprintw(win, y, x, "%s", text.c_str());
        wrefresh(win);
        sleep_ms(30);
    }
}

void Animations::fadeOut(WINDOW* win, int y, int x, const std::string& text, int color_pair) {
    for(int i = 100; i >= 0; i -= 10) {
        wattron(win, COLOR_PAIR(color_pair) | (i > 50 ? A_BOLD : 0));
        mvwprintw(win, y, x, "%s", text.c_str());
        wrefresh(win);
        sleep_ms(30);
    }
    mvwprintw(win, y, x, "%-*s", (int)text.length(), " ");
}

void Animations::matrixRain(WINDOW* win, int duration_ms) {
    int height, width;
    getmaxyx(win, height, width);
    
    srand(time(nullptr));
    
    wattron(win, COLOR_PAIR(Colors::ONLINE));
    for(int frame = 0; frame < duration_ms / 50; frame++) {
        for(int i = 0; i < width / 3; i++) {
            int x = rand() % width;
            int y = rand() % height;
            char c = 33 + (rand() % 94); // Printable characters
            
            mvwprintw(win, y, x, "%c", c);
        }
        wrefresh(win);
        sleep_ms(50);
        werase(win);
    }
    wattroff(win, COLOR_PAIR(Colors::ONLINE));
}

void Animations::connectingAnimation(WINDOW* win, int y, int x, const std::string& host, int port) {
    const char* frames[] = {
        "◴ Connecting",
        "◷ Connecting",
        "◶ Connecting",
        "◵ Connecting"
    };
    
    for(int i = 0; i < 20; i++) {
        wattron(win, COLOR_PAIR(Colors::HIGHLIGHT));
        mvwprintw(win, y, x, "%s to %s:%d", frames[i % 4], host.c_str(), port);
        wrefresh(win);
        sleep_ms(150);
    }
    wattroff(win, COLOR_PAIR(Colors::HIGHLIGHT));
}