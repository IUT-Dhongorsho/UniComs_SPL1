#include "NetworkManager.hpp"
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

NetworkManager::NetworkManager(int fd) : fd(fd) {}

NetworkManager::~NetworkManager() {
    stop();
}

void NetworkManager::start(std::function<void(const std::string&)> onMessage) {
    running = true;
    std::thread([this, onMessage]() {
        while (running) {
            std::string line = recvLine(fd);
            if (line.empty()) {
                if (running) {
                    onMessage("[disconnected from server]");
                }
                running = false;
                break;
            }
            onMessage(line);
        }
    }).detach();
}

void NetworkManager::stop() {
    if (running) {
        running = false;
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
}

bool NetworkManager::send(const std::string& line) {
    return sendLine(fd, line);
}
