#include "Server.h"
#include "../network/Listener.h"
#include "../core/ClientHandler.h"
#include "../../common/utils/Logger.h"

Server::Server(int port) : port(port), running(false) {}

void Server::start() {
    running = true;
    Listener listener(port);

    Logger::info("Server listening on port " + std::to_string(port));

    while (running) {
        int clientFd = listener.acceptClient();
        if (clientFd < 0) continue;

        Logger::info("Client connected");

        clientThreads.emplace_back([this, clientFd]() {
            ClientHandler handler(clientFd, &router);
            handler.handle();
        });        
    }
}

void Server::stop() {
    running = false;
}
