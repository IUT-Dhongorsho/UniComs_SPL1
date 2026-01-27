#include "server.h"

void joinChatRoom(const std::string roomName, const int clientSock, sockToName &name, chatRoomToSockList &chatRooms)
{
    auto chatRoomItr = chatRooms.find(roomName);
    bool chatRoomExists = (chatRoomItr != chatRooms.end());

    if (!chatRoomExists)
    {
        chatRooms[roomName] = {clientSock};
    }
    else
    {
        chatRoomItr->second.insert(clientSock);
    }

    std::stringstream ss;
    ss << "%%" << "join" << "%%" << roomName;
    std::string res = ss.str();

    if (chatRoomExists)
    {
        std::stringstream notification;
        notification << "info" << "%%" << name[clientSock] << "%%" << "joined " << roomName;
        
        //Broadcast
    }
}