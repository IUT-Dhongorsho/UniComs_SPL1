#include "Socket.h"
#include <unistd.h>

Socket::Socket(int fd) : fd(fd) {}

bool Socket::sendBytes(const std::vector<char>& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = write(fd, data.data() + sent, data.size() - sent);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

std::vector<char> Socket::receiveBytes() {
    char buffer[4096];
    ssize_t n = read(fd, buffer, sizeof(buffer));
    if (n <= 0) return {};
    return std::vector<char>(buffer, buffer + n);
}
