#include "ClientHandler.h"
#include "../network/Socket.h"
#include "../../common/utils/Logger.h"
#include "../../common/utils/Serializer.h"

ClientHandler::ClientHandler(int clientSocket, MessageRouter* router)
    : clientSocket(clientSocket), router(router) {}

void ClientHandler::handle() {
    Socket socket(clientSocket);

    while (true) {
        auto data = socket.receiveBytes();
        if (data.empty()) break;

        Packet packet = Serializer::deserialize(data);

        switch (packet.type) {

            case PacketType::LOGIN:
            username = packet.sender;
            router->registerClient(username, clientSocket);
            Logger::info(username + " logged in");
            break;
        

        case PacketType::DIRECT_MESSAGE:
            Logger::info("DM from " + username + " to " + packet.recipient);
            router->routeDirect(packet.recipient, data);
            break;

        default:
            Logger::info("Unhandled packet type");
        }
    }

    if (!username.empty()) {
        router->unregisterClient(username);
    }
}
