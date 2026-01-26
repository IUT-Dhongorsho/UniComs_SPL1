#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "../network/Socket.h"
#include "../handlers/MessageHandler.h"

class Client {
public:
    Client(const std::string& host, int port);

    bool connectToServer();
    void start();
    void stop();

    Socket& getSocket();

private:
    std::string host;
    int port;
    bool running;

    Socket socket;
    MessageHandler messageHandler;
};

#endif
