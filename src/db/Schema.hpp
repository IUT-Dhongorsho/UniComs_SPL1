#ifndef SCHEMA_H
#define SCHEMA_H
#include "Database.hpp"
#include <string>

struct User
{
    std::string id;
    std::string name;
    std::string email;
    std::string password;
    bool isActive;
    bool isDeleted;

    static Schema schema()
    {
        return {
            "User",
            {{"id", FieldType::String},
             {"name", FieldType::String},
             {"email", FieldType::String},
             {"password", FieldType::String},
             {"isActive", FieldType::Boolean},
             {"isDeleted", FieldType::Boolean}}};
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
};

#endif