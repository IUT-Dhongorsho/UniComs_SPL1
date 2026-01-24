#include "ConnectingSocket.hpp"

// Constructor
HDE::ConnectingSocket::ConnectingSocket(int domain, int service, int protocol, int port, u_long interface) : SimpleSocket(domain, service, protocol, port, interface) {
    setConnection(connectToNetwork(getSock(), getAddress()));
    testConnection(getConnection());
}

int HDE::ConnectingSocket::connectToNetwork(int sock, struct sockaddr_in address){
    return connect(sock, (struct sockaddr *)&address, sizeof(address));
}
