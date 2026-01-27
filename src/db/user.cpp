#include "User.h"

// Define static members
const std::vector<std::string> User::Keys = {"name", "password", "status"};
const size_t User::SerializedSize = 64;

// Default constructor
User::User() : Entry(SerializedSize, Keys) {}

// Constructor with values
User::User(const Entry::Values &values) : Entry(SerializedSize) {
    if (values.size() != Keys.size()) {
        throw std::invalid_argument(
            "Failed to create User: values count must match keys count");
    }
    
    for (size_t i = 0; i < Keys.size(); ++i) {
        insert(Keys[i], values[i]);
    }
}

// De-serialize constructor
User::User(const std::string &str) : Entry(SerializedSize, str) {}

// Convenience getters
std::string User::getName() const { return get("name"); }
std::string User::getPassword() const { return get("password"); }
std::string User::getStatus() const { return get("status"); }

// Convenience setters
bool User::setName(const std::string &name) { return set("name", name); }
bool User::setPassword(const std::string &password) { return set("password", password); }
bool User::setStatus(const std::string &status) { return set("status", status); }