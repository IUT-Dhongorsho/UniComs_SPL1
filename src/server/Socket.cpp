#include "server.h"
#include <stdexcept>
#include <unistd.h>

int serverSocket()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("Failed to create socket");

    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    return fd;
}

void serverBind(int fd, int port)
{
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error("Bind failed");
}

void serverListen(int fd, int backlog)
{
    if (listen(fd, backlog) < 0)
        throw std::runtime_error("Listen failed");
}

int serverAccept(int fd)
{
    struct sockaddr_in clientAddr{};
    socklen_t len = sizeof(clientAddr);
    return accept(fd, reinterpret_cast<struct sockaddr *>(&clientAddr), &len);
}