#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <string>
#include <unordered_map>

class AuthManager {
public:
    bool signup(const std::string& username, const std::string& password);
    bool login(const std::string& username, const std::string& password);
    void logout(const std::string& username);

private:
    std::unordered_map<std::string, std::string> users; // username -> password hash
    std::unordered_map<std::string, bool> sessions;     // username -> online/offline

    std::string hashPassword(const std::string& password);
};

#endif
