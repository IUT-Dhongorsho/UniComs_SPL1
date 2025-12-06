#ifndef ListeningSocket_hpp
#define ListeningSocket_hpp

#include "BindingSocket.hpp"

namespace HDE{
    class ListeningSocket : public BindingSocket{
        private:
            int backlog;
            int listening;
        public:
            ListeningSocket(int domain, int service, int protocol, int port, u_long interface, int bklg);
            
            void startListening();
    };
}

#endif