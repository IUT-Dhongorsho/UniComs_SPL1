#include "./server.h"
#include <thread>
#include <stdexcept>
#include <unistd.h>

int main(int argc, char *argv[])
{
    ServerState state;

    // Initialize database and ensure all tables exist
    state.db.init();
    state.db.create<User>();
    state.db.create<Message>();
    state.db.create<ChatRoom>();
    state.db.create<ChatRoomMember>();

    int port = 8080;
    if (argc >= 2)
    {
        port = std::stoi(argv[1]);
    }

    int sockfd = serverSocket();
    serverBind(sockfd, port);
    serverListen(sockfd, 10);

    std::cout << "[server] Listening on port " << port << "\n";

    while (true)
    {
        int clientFd = serverAccept(sockfd);
        if (clientFd < 0)
        {
            std::cerr << "[server] Accept failed\n";
            continue;
        }

        // Register session
        {
            std::lock_guard<std::mutex> lock(state.mtx);
            state.sessions[clientFd] = ClientSession{clientFd, "", "", false};
        }

        std::cout << "[server] New connection fd=" << clientFd << "\n";

        // Spawn a thread per client
        std::thread([clientFd, &state]()
                    { handleClient(clientFd, state); })
            .detach();
    }

    close(sockfd);
    return 0;
}