#include "server.h"

void leaveChatRoom(const std::string roomName, const int clientSock, const sockToName &names, chatRoomToSockList &chatRooms)
{
    auto roomItr = chatRooms.find(roomName);
    bool roomExists = (roomItr != chatRooms.end());

    if (!roomExists)
    {
        std::stringstream error;
        error << "ERROR" << "%%" << "leave" << "%%" << "cannot leave #" << roomName;
        send(clientSock, error.str().c_str(), error.str().size(), 0);
        return;
    }

    std::string clientName = names.at(clientSock);

    auto &roomMembers = chatRooms[roomName];
    roomMembers.erase(clientSock);

    if (roomMembers.empty())
    {
        chatRooms.erase(roomName);
    }

    std::stringstream response;
    response << "%%" << "leave" << "%%" << roomName;
    send(clientSock, response.str().c_str(), response.str().size(), 0);

    if (chatRooms.count(roomName))
    {
        std::stringstream notification;
        notification << "info" << "%%" << clientName << "#" << roomName << "%%" << "left chatroom";
        // broadcast(chatRooms[roomName], clientSock, notification.str());
    }
}