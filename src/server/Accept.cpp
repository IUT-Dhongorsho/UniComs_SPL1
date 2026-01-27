#include "server.h"

int Accept(int sockfd)
{
    struct sockaddr_in clientAddr;
    socklen_t len = static_cast<socklen_t>(sizeof(clientAddr));
    const int clientSockfd = accept(
        sockfd,
        reinterpret_cast<struct sockaddr *>(&clientAddr),
        &len);

    return clientSockfd;
}