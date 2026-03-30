#pragma once
#include "../Schema.h"
#include <string>
#include <vector>
#include <stdexcept>

struct User
{
    std::string id;
    std::string username;
    std::string password; // stored as plain text for simplicity

    static Schema schema()
    {
        return {"User",
                {{"id", "string", true, true},
                 {"username", "string", false, true},
                 {"password", "string"}}};
    }

    std::vector<std::string> serialize() const
    {
        return {id, username, password};
    }

    static User deserialize(const std::vector<std::string> &row)
    {
        if (row.size() != 3)
            throw std::runtime_error("Invalid row size for User");
        return {row[0], row[1], row[2]};
    }
};