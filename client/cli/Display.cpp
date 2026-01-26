#include "Display.h"
#include <iostream>

void Display::info(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void Display::message(const std::string& msg) {
    std::cout << msg << std::endl;
}
