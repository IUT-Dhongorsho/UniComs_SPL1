#pragma once
#include <string>
#include <atomic>
#include <functional>
#include "../utils/Utils.h"

class NetworkManager {
public:
    NetworkManager(int fd);
    ~NetworkManager();

    void start(std::function<void(const std::string&)> onMessage);
    void stop();

    bool send(const std::string& line);
    int getFd() const { return fd; }

private:
    int fd;
    std::atomic<bool> running{false};
};
