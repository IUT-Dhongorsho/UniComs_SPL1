#include "server.h"

void Listen(int sockfd, int backlog)
{
    const int res = listen(sockfd, backlog);
    if (res < 0)
    {
        throw std::runtime_error("Failed to listen");
    }
}