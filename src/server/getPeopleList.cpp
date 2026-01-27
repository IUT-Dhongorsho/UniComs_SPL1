#include "server.h"
#include <sstream>

std::string getPeopleList(const std::string &roomName, const sockToName &names, const chatRoomToSockList &chatRooms)
{
    // Check if room exists
    auto roomItr = chatRooms.find(roomName);
    if (roomItr == chatRooms.end())
    {
        return "%%error%%Room '" + roomName + "' does not exist";
    }

    std::stringstream peopleList;

    peopleList << "%%people%%" << roomName << "\n";

    for (const auto &sock : roomItr->second)
    {
        auto nameItr = names.find(sock);
        if (nameItr != names.end() && !nameItr->second.empty())
        {
            std::string peerInfo = getClientInfo(sock);
            peopleList << nameItr->second << " <" << peerInfo << ">\n";
        }
    }

    return peopleList.str();
}