#include "Listener.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

Listener::Listener(int port) : port(port) {
    serverFd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(serverFd, (sockaddr*)&addr, sizeof(addr));
    listen(serverFd, 10);
}

int Listener::acceptClient() {
    return accept(serverFd, nullptr, nullptr);
}
