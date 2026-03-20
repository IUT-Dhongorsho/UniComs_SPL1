#pragma once
#include <ncurses.h>
#include <functional>
#include <vector>
#include "../Colors.hpp"

struct Setting {
    std::string name;
    std::vector<std::string> options;
    int current;
};

class SettingsScreen {
private:
    WINDOW* window;
    std::vector<Setting> settings;
    int selected;
    bool isVisible;
    
public:
    SettingsScreen();
    ~SettingsScreen();
    
    void setWindow(WINDOW* win);
    void show();
    void hide();
    void render();
    bool handleKey(int key);
    
    std::function<void(Colors::Theme)> onThemeChange;
    std::function<void()> onBack;
};