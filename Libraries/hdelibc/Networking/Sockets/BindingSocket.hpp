#ifndef BindingSocket_hpp
#define BindingSocket_hpp

#include "SimpleSocket.hpp"

namespace HDE{
    class BindingSocket : public SimpleSocket{
        public:
            // Constructor
            BindingSocket(int domain, int service, int protocol, int port, u_long interface);

            // Virtual function from parent class
            int connectToNetwork(int sock, struct sockaddr_in address);
    };
}

#endif