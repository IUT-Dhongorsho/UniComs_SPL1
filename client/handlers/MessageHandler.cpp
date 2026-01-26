#include "MessageHandler.h"
#include "../../common/utils/Serializer.h"
#include "../cli/Display.h"

void MessageHandler::listen(Socket& socket) {
    while (true) {
        auto data = socket.receiveBytes();
        if (data.empty()) break;

        Packet packet = Serializer::deserialize(data);

        Display::message(
            packet.sender + ": " +
            std::string(packet.payload.begin(), packet.payload.end())
        );
    }
}

