#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "../network/Socket.h"

class MessageHandler {
public:
    void listen(Socket& socket);
};

#endif
