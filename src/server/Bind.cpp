#include "server.h"
#include "../utils/utils.h"

void Bind(int sockfd, int port)
{
    struct sockaddr_in serverAddr;

    memorySet(&serverAddr, 0, sizeof(serverAddr));

    // for testing purposes
    const int reuse = 1;
    if (setsockopt(
            sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            reinterpret_cast<const char *>(&reuse),
            sizeof(reuse)) < 0)
    {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = hostToNetShort(port);

    int res = bind(sockfd,
                   reinterpret_cast<struct sockaddr *>(&serverAddr),
                   sizeof serverAddr);
    if (res < 0)
    {
        throw std::runtime_error("Error in binding socket");
    }
}