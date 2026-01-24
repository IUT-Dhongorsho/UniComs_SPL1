#include "BindingSocket.hpp"

// Constructor
HDE::BindingSocket::BindingSocket(int domain, int service, int protocol, int port, u_long interface) : SimpleSocket(domain, service, protocol, port, interface){
    setConnection(connectToNetwork(getSock(), getAddress()));
    testConnection(getConnection());
}

// Defination of connectToNetwork virtual function
int HDE::BindingSocket::connectToNetwork(int sock, struct sockaddr_in address){
    return bind(sock, (struct sockaddr *)&address, sizeof(address));
}
