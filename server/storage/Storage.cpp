#include "Storage.h"
#include <fstream>

Storage::Storage(const std::string& basePath) : basePath(basePath) {}

void Storage::saveUser(const std::string& username, const std::string& hash) {
    std::lock_guard<std::mutex> lock(fileLock);
    std::ofstream file(basePath + "/users.db", std::ios::app);
    file << username << ":" << hash << "\n";
}

void Storage::saveMessage(const std::string& record) {
    std::lock_guard<std::mutex> lock(fileLock);
    std::ofstream file(basePath + "/messages.db", std::ios::app);
    file << record << "\n";
}

void Storage::saveGroup(const std::string& record) {
    std::lock_guard<std::mutex> lock(fileLock);
    std::ofstream file(basePath + "/groups.db", std::ios::app);
    file << record << "\n";
}
