#ifndef GROUP_MANAGER_H
#define GROUP_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

class GroupManager {
public:
    bool createGroup(const std::string& groupName);
    bool joinGroup(const std::string& groupName, const std::string& username);
    bool leaveGroup(const std::string& groupName, const std::string& username);

    std::vector<std::string> getMembers(const std::string& groupName);

private:
    std::unordered_map<std::string, std::vector<std::string>> groups;
};

#endif
