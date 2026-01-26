#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <thread>
#include "../routing/MessageRouter.h"

class Server {
public:
    explicit Server(int port);
    void start();
    void stop();

private:
    int port;
    bool running;
    std::vector<std::thread> clientThreads;
    MessageRouter router;
    void acceptClients();
};

#endif
