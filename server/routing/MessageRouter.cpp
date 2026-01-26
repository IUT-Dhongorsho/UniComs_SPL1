#include "MessageRouter.h"
#include "../network/Socket.h"

void MessageRouter::registerClient(const std::string& username, int fd) {
    activeClients[username] = fd;
}

void MessageRouter::unregisterClient(const std::string& username) {
    activeClients.erase(username);
}

void MessageRouter::routeDirect(const std::string& to, const std::vector<char>& data) {
    if (activeClients.count(to)) {
        Socket sock(activeClients[to]);
        sock.sendBytes(data);
    }
}
