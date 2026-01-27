#include "./server.h"

void broadcast(const sockList &clients, const int currentClientFd, const std::string &msg)
{
    for (int clientSock : clients)
    {
        if (clientSock != currentClientFd)
        {
            send(clientSock, msg.c_str(), msg.length(), 0);
        }
    }
}
