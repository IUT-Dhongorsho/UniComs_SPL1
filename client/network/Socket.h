#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <vector>

class Socket {
public:
    explicit Socket(int fd);

    bool sendBytes(const std::vector<char>& data);
    std::vector<char> receiveBytes();

private:
    int fd;
};

#endif
