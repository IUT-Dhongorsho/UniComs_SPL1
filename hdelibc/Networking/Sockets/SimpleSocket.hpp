#ifndef SimpleSocket_hpp
#define SimpleSocket_hpp

#include <sys/socket.h>
#include <netinet/in.h>
#include "NetworkUtils.hpp"

// struct sockaddr_in {
//     unsigned short sin_family;
//     unsigned short sin_port;
//     struct in_addr {
//         unsigned long s_addr;
//     } sin_addr;
//     char sin_zero[8];
// };

namespace HDE{
    class SimpleSocket{
        private:
            int sock;
            int connection;
            struct sockaddr_in address;
        public:
            // Constructor
            SimpleSocket(int domain, int service, int protocol, int port, u_long interface);

            // Virtual function to connect to network
            virtual int connectToNetwork(int sock, struct sockaddr_in address) = 0;

            // Function to teste connections and sockets
            void testConnection(int);

            // Getter functions
            struct sockaddr_in getAddress();
            int getSock();
            int getConnection();

            // Setter functions
            void setConnection(int con);
    };
}

#endif
