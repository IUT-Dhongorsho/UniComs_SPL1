#ifndef ConnectingSocket_hpp
#define ConnectingSocket_hpp

#include "SimpleSocket.hpp"

namespace HDE{
    class ConnectingSocket : public SimpleSocket{
        public:
            // Constructor
            ConnectingSocket(int domain, int service, int protocol, int port, u_long interface);
            
            // Virtual function from parent
            int connectToNetwork(int sock, struct sockaddr_in address);
    };
}

#endif