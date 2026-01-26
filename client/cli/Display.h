#ifndef DISPLAY_H
#define DISPLAY_H

#include <string>

class Display {
public:
    static void info(const std::string& msg);
    static void message(const std::string& msg);
};

#endif
