// Default constructor for socket
#include "SimpleSocket.hpp"

HDE::SimpleSocket::SimpleSocket(int domain, int service, int protocol, int port, u_long interface){
    // Define all the addresses
    address.sin_family = domain;
    address.sin_port = HDE::htonsCustom(port);
    address.sin_addr.s_addr = HDE::htonlCustom(interface);

    // Initialize sin_zero to zeros
    for(int i = 0; i < 8; i++){
        address.sin_zero[i] = 0;
    }

    // Establish a socket
    sock = socket(domain, service, protocol);
    testConnection(sock);

}

// Test if conncetection successful (virtual function)
void HDE::SimpleSocket::testConnection(int test){
    // Confirm if the socket or connection has been properly established
    if(test<0){
        HDE::printError("Failed to connect to socket");
        exit(EXIT_FAILURE);
    }
}

// Getter functions
struct sockaddr_in HDE::SimpleSocket::getAddress(){
    return address;
}

int HDE::SimpleSocket::getSock(){
    return sock;
}

int HDE::SimpleSocket::getConnection(){
    return connection;
}

// Setter functions
void HDE::SimpleSocket::setConnection(int con){
    connection = con;
}