#include "Server.h"
#include <string>

std::string getClientInfo(const int clientSock)
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int result = getpeername(clientSock, reinterpret_cast<struct sockaddr *>(&clientAddr), &addrLen);

    if (result != 0)
    {
        return "unknown";
    }

    char ipBuffer[INET_ADDRSTRLEN] = {0};
    const char *ipStr = inet_ntop(AF_INET,
                                  &clientAddr.sin_addr,
                                  ipBuffer,
                                  sizeof(ipBuffer));

    if (ipStr == nullptr)
    {
        return "unknown";
    }

    uint16_t port = ntohs(clientAddr.sin_port);

    return std::string(ipBuffer) + ":" + std::to_string(port);
}