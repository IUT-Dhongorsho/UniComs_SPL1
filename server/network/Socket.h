#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H

#include <vector>

class Socket {
public:
    explicit Socket(int fd);
    int getFd() const;

    bool sendBytes(const std::vector<char>& data);
    std::vector<char> receiveBytes();

private:
    int fd;
};

#endif
