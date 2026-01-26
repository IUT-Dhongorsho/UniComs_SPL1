#include "AuthManager.h"
#include <functional>

std::string AuthManager::hashPassword(const std::string& password) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

bool AuthManager::signup(const std::string& username, const std::string& password) {
    if (users.find(username) != users.end()) {
        return false;
    }
    users[username] = hashPassword(password);
    sessions[username] = false;
    return true;
}

bool AuthManager::login(const std::string& username, const std::string& password) {
    if (users.find(username) == users.end()) {
        return false;
    }
    if (users[username] != hashPassword(password)) {
        return false;
    }
    sessions[username] = true;
    return true;
}

void AuthManager::logout(const std::string& username) {
    sessions[username] = false;
}
