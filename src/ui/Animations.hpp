#pragma once
#include <ncurses.h>
#include <string>
#include <chrono>
#include <thread>

class Animations {
public:
    // Loading spinner animations
    static void loadingSpinner(WINDOW* win, int y, int x, const std::string& message, int duration_ms = 2000);
    
    // Typing indicator animation
    static void typingIndicator(WINDOW* win, int y, int x, const std::string& username, int cycles = 10);
    
    // Progress bar animation
    static void progressBar(WINDOW* win, int y, int x, int width, int progress, const std::string& label = "");
    
    // Wave animation for voice
    static void waveAnimation(WINDOW* win, int y, int x, int width, int amplitude);
    
    // Pulse animation for notifications
    static void pulse(WINDOW* win, int y, int x, const std::string& text, int color_pair, int pulses = 3);
    
    // Smooth scrolling text
    static void scrollText(WINDOW* win, int y, int x, const std::string& text, int width, int speed_ms = 50);
    
    // Fade in/out effect
    static void fadeIn(WINDOW* win, int y, int x, const std::string& text, int color_pair);
    static void fadeOut(WINDOW* win, int y, int x, const std::string& text, int color_pair);
    
    // Matrix rain effect
    static void matrixRain(WINDOW* win, int duration_ms = 2000);
    
    // Connection status animation
    static void connectingAnimation(WINDOW* win, int y, int x, const std::string& host, int port);
    
private:
    static void sleep_ms(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
};