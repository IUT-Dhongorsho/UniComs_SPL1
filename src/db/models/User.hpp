#pragma once
#include "../Schema.hpp"
#include <string>
#include <vector>
#include <stdexcept>

struct User
{
    std::string id;
    std::string name;
    std::string email;
    std::string password;
    bool isActive;
    bool isDeleted;

    // Provide schema for your template engine
    static Schema schema()
    {
        return {
            "User",
            {{"id", "string", true, true}, // primary + unique
             {"name", "string"},
             {"email", "string", false, true}, // unique
             {"password", "string"},
             {"isActive", "boolean"},
             {"isDeleted", "boolean"}}};
    }

    std::vector<std::string> serialize() const
    {
        return {
            id,
            name,
            email,
            password,
            isActive ? "1" : "0",
            isDeleted ? "1" : "0"};
    }

    static User deserialize(const std::vector<std::string> &row)
    {
        if (row.size() != 6)
            throw std::runtime_error("Invalid row size for User");

        User u;
        u.id = row[0];
        u.name = row[1];
        u.email = row[2];
        u.password = row[3];
        u.isActive = (row[4] == "1");
        u.isDeleted = (row[5] == "1");
        return u;
    }
};
