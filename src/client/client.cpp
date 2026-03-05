#include "client.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <unistd.h>

// Background thread: prints messages received from server
static void receiveLoop(int fd, std::atomic<bool> &running)
{
    while (running)
    {
        std::string line = recvLine(fd);
        if (line.empty())
        {
            std::cout << "\n[disconnected from server]\n";
            running = false;
            break;
        }
        std::cout << "\r" << line << "\n> " << std::flush;
    }
}

void runClient(int fd)
{
    std::atomic<bool> running{true};
    std::thread receiver(receiveLoop, fd, std::ref(running));

    std::cout << "Connected. Type commands or 'quit' to exit.\n> " << std::flush;

    std::string line;
    while (running && std::getline(std::cin, line))
    {
        if (line == "quit" || line == "exit")
            break;

        if (!sendLine(fd, line))
        {
            std::cerr << "[error] Failed to send\n";
            break;
        }
        std::cout << "> " << std::flush;
    }

    running = false;
    shutdown(fd, SHUT_RDWR);
    close(fd);
    receiver.join();
}

int main(int argc, char *argv[])
{
    std::string host = "127.0.0.1";
    int port = 8080;

    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);

    int fd = clientConnect(host, port);
    if (fd < 0)
    {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        return 1;
    }

    runClient(fd);
    return 0;
}