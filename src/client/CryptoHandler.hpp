#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "session.h"

class CryptoHandler {
public:
    CryptoHandler(int fd, std::unordered_map<std::string, CryptoSession>& sessions);

    bool handleDHInit(const std::string& line);
    bool handleDHReply(const std::string& line);

    std::string encrypt(const std::string& peer, const std::string& msg);
    std::string decrypt(const std::string& peer, const std::string& msg);
    
    void initiateSession(const std::string& peer);
    bool isSessionReady(const std::string& peer);

private:
    int fd;
    std::unordered_map<std::string, CryptoSession>& sessionStore;
};
