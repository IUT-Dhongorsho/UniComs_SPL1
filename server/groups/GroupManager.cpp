#include "GroupManager.h"

bool GroupManager::createGroup(const std::string& groupName) {
    if (groups.find(groupName) != groups.end()) {
        return false;
    }
    groups[groupName] = {};
    return true;
}

bool GroupManager::joinGroup(const std::string& groupName, const std::string& username) {
    if (groups.find(groupName) == groups.end()) {
        return false;
    }
    groups[groupName].push_back(username);
    return true;
}

bool GroupManager::leaveGroup(const std::string& groupName, const std::string& username) {
    auto& members = groups[groupName];
    members.erase(
        std::remove(members.begin(), members.end(), username),
        members.end()
    );
    return true;
}

std::vector<std::string> GroupManager::getMembers(const std::string& groupName) {
    return groups[groupName];
}
