#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <mutex>

class Storage {
public:
    explicit Storage(const std::string& basePath);

    void saveUser(const std::string& username, const std::string& hash);
    void saveMessage(const std::string& record);
    void saveGroup(const std::string& record);

private:
    std::string basePath;
    std::mutex fileLock;
};

#endif
