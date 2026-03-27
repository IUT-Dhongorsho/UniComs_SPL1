#pragma once
#include "../Schema.h"
#include <string>
#include <vector>
#include <stdexcept>

// Join table: which users are members of which chat rooms.
// Composite "primary key" is represented as "roomId:userId".
struct ChatRoomMember
{
    std::string id; // roomId:userId
    std::string roomId;
    std::string userId;

    static Schema schema()
    {
        return {"ChatRoomMember",
                {{"id", "string", true, true},
                 {"roomId", "string"},
                 {"userId", "string"}}};
    }

    std::vector<std::string> serialize() const
    {
        return {id, roomId, userId};
    }

    static ChatRoomMember deserialize(const std::vector<std::string> &row)
    {
        if (row.size() != 3)
            throw std::runtime_error("Invalid row size for ChatRoomMember");
        return {row[0], row[1], row[2]};
    }
};