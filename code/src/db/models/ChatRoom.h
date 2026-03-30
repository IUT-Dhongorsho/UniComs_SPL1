#pragma once
#include "../Schema.h"
#include <string>
#include <vector>
#include <stdexcept>

struct ChatRoom
{
    std::string id;
    std::string name;
    std::string creatorId;
    std::string password; // hashed password

    static Schema schema()
    {
        return {"ChatRoom",
                {{"id", "string", true, true},
                 {"name", "string", false, true},
                 {"creatorId", "string"},
                 {"password", "string"}}};
    }

    std::vector<std::string> serialize() const
    {
        return {id, name, creatorId, password};
    }

    static ChatRoom deserialize(const std::vector<std::string> &row)
    {
        if (row.size() != 4)
            throw std::runtime_error("Invalid row size for ChatRoom");
        return {row[0], row[1], row[2], row[3]};
    }
};