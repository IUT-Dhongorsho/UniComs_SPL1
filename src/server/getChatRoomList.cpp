#include "server.h"
#include <sstream>

std::string getChatroomList(const chatRoomToSockList &chatRooms)
{
    std::stringstream chatRoomsStream;
    chatRoomsStream << "%%" << "Chat List" << "%%" << "\n";

    for (const auto &room : chatRooms)
    {
        const std::string &roomName = room.first;
        size_t memberCount = room.second.size();

        chatRoomsStream << roomName << " <" << memberCount << ">\n";
    }

    // Return the complete string
    return chatRoomsStream.str();
}