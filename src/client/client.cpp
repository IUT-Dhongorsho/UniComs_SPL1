#include "ClientController.h"
#include "Client.h"
#include <iostream>

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

    ClientController controller(fd);
    controller.run();

    return 0;
}
