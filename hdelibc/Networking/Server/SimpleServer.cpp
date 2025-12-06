#include "SimpleServer.hpp"

HDE::SimpleServer::SimpleServer(int domain, int service, int protocol, int port, u_long interface, int bklg){
    socket = new ListeningSocket(domain, service, protocol, port, interface, bklg);
}

HDE::ListeningSocket* HDE::SimpleServer::getSocket(){
    return socket;
}

