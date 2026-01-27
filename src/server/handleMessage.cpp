#include "server.h"
#include <sstream>

void handleMsg(const int currentClientSock, const chatRoomToSockList &chatRooms, const sockToName &clients, std::string msg)
{
    // Find first delimiter
    size_t firstDelim = msg.find("%%");
    if (firstDelim == std::string::npos)
    {
        // Invalid format
        std::string errorMsg = "ERROR%%SERVER%%Invalid message format";
        send(currentClientSock, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // Find second delimiter
    size_t secondDelim = msg.find("%%", firstDelim + 2);
    if (secondDelim == std::string::npos)
    {
        // No message content
        std::string errorMsg = "ERROR%%SERVER%%No message content";
        send(currentClientSock, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    std::string chatRoomName = msg.substr(0, firstDelim);
    std::string senderInfo = msg.substr(firstDelim + 2, secondDelim - firstDelim - 2);
    std::string text = msg.substr(secondDelim + 2);

    // Check if chatroom exists
    if (chatRooms.count(chatRoomName) == 0)
    {
        std::string errorMsg = "ERROR%%SERVER%%Chatroom " + chatRoomName + " doesn't exist";
        send(currentClientSock, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // Check if user is in the chatroom
    const auto &roomMembers = chatRooms.at(chatRoomName);
    if (roomMembers.count(currentClientSock) == 0)
    {
        std::string errorMsg = "ERROR%%SERVER%%You must join " + chatRoomName + " first";
        send(currentClientSock, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    std::stringstream msgStream;
    msgStream << "MSG%%" << senderInfo << "#" << chatRoomName << "%%" << text;
    std::string msgToSend = msgStream.str();

    if (!text.empty() && text[0] == '@')
    {
        // Extract username after @
        size_t spacePos = text.find(' ');
        std::string receiverName;

        if (spacePos == std::string::npos)
        {
            receiverName = text.substr(1);
        }
        else
        {
            receiverName = text.substr(1, spacePos - 1);
        }

        receiverName.erase(receiverName.find_last_not_of(" \t\n\r\f\v") + 1);
        receiverName.erase(0, receiverName.find_first_not_of(" \t\n\r\f\v"));

        int receiverSock = -1;
        for (const auto &[sock, name] : clients)
        {
            if (name == receiverName)
            {
                receiverSock = sock;
                break;
            }
        }

        if (receiverSock != -1 && roomMembers.count(receiverSock) != 0)
        {
            send(receiverSock, msgToSend.c_str(), msgToSend.size(), 0);

            std::string confirmMsg = "MSG%%SERVER%%Private message sent to @" + receiverName;
            send(currentClientSock, confirmMsg.c_str(), confirmMsg.size(), 0);
        }
        else
        {
            std::string errorMsg = "ERROR%%SERVER%%User @" + receiverName +
                                   " not found or not in this room";
            send(currentClientSock, errorMsg.c_str(), errorMsg.size(), 0);
        }
        return;
    }

    for (int clientSock : roomMembers)
    {
        if (clientSock != currentClientSock)
        {
            send(clientSock, msgToSend.c_str(), msgToSend.size(), 0);
        }
    }
}