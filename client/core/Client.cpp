#include "Client.h"
#include "../cli/CommandParser.h"
#include "../cli/Display.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>

Client::Client(const std::string& host, int port)
    : host(host), port(port), running(false), socket(-1) {}

bool Client::connectToServer() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        Display::info("Connection failed");
        return false;
    }

    socket = Socket(fd);
    Display::info("Connected to server");

    std::thread receiver(&MessageHandler::listen, &messageHandler, std::ref(socket));
    receiver.detach();

    return true;
}

void Client::start() {
    running = true;
    CommandParser parser(*this);

    while (running) {
        parser.readAndExecute();
    }
}

void Client::stop() {
    running = false;
}

Socket& Client::getSocket() {
    return socket;
}
