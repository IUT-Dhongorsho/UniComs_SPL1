#pragma once
#include "../Schema.hpp"
#include <string>
#include <vector>
#include <stdexcept>

// Unified message model for both DMs and group chat.
// For DMs:    type="dm",    roomId="" (unused), toId=recipient user id
// For group:  type="group", roomId=chat room id, toId=""
struct Message
{
    std::string id;
    std::string type;    // "dm" | "group"
    std::string fromId;  // sender user id
    std::string toId;    // recipient user id (DM only)
    std::string roomId;  // chat room id (group only)
    std::string content;
    std::string timestamp;

    static Schema schema()
    {
        return {"Message",
                {{"id", "string", true, true},
                 {"type", "string"},
                 {"fromId", "string"},
                 {"toId", "string"},
                 {"roomId", "string"},
                 {"content", "string"},
                 {"timestamp", "string"}}};
    }

    std::vector<std::string> serialize() const
    {
        return {id, type, fromId, toId, roomId, content, timestamp};
    }

    static Message deserialize(const std::vector<std::string> &row)
    {
        if (row.size() != 7)
            throw std::runtime_error("Invalid row size for Message");
        return {row[0], row[1], row[2], row[3], row[4], row[5], row[6]};
    }
};