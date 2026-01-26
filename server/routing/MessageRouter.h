#pragma once

#include <string>
#include <vector>
#include <unordered_map>
class MessageRouter {
private:
    std::unordered_map<std::string, int> activeClients;

public:
    void registerClient(const std::string& username, int fd);
    void unregisterClient(const std::string& username);
    void routeDirect(const std::string& to, const std::vector<char>& data);
};
