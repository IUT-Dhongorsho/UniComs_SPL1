#pragma once
#include "../Schema.hpp"
#include <string>
#include <vector>
#include <stdexcept>

struct ChatRoom
{
    std::string id;
    std::string name;
    std::string creatorId;

    static Schema schema()
    {
        return {"ChatRoom",
                {{"id", "string", true, true},
                 {"name", "string", false, true},
                 {"creatorId", "string"}}};
    }

    std::vector<std::string> serialize() const
    {
        return {id, name, creatorId};
    }

    static ChatRoom deserialize(const std::vector<std::string> &row)
    {
        if (row.size() != 3)
            throw std::runtime_error("Invalid row size for ChatRoom");
        return {row[0], row[1], row[2]};
    }
};