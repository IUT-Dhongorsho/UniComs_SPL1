#ifndef TestServer_hpp
#define TestServer_hpp

#include "SimpleServer.hpp"
#include <unistd.h>

namespace HDE{
    class TestServer : public SimpleServer{
        private:
            char buffer[30000];
            int newSocket;
            void accepter();
            void handler();
            void responder();
        public:
            TestServer();
            void launch();
    };
}

#endif