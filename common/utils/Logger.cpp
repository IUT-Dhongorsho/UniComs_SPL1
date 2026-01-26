#include "Logger.h"
#include <iostream>
#include <ctime>

static std::string timestamp() {
    std::time_t now = std::time(nullptr);
    return std::string(std::ctime(&now));
}

void Logger::info(const std::string& msg) {
    std::cout << "[INFO] " << timestamp() << " " << msg << std::endl;
}

void Logger::error(const std::string& msg) {
    std::cerr << "[ERROR] " << timestamp() << " " << msg << std::endl;
}
