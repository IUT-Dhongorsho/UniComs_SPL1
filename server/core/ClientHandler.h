#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H
#include <string>
#include "../routing/MessageRouter.h"

class ClientHandler {
public:
    ClientHandler(int clientSocket, MessageRouter* router);
    void handle();

private:
    int clientSocket;
    MessageRouter* router;
    std::string username;
};


#endif
