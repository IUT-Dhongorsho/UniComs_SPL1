#pragma once
#include "Terminal.hpp"
#include "NetworkManager.hpp"
#include "CryptoHandler.hpp"
#include "FileHandler.hpp"
#include "VoiceHandler.hpp"
#include <unordered_map>
#include <string>

class ClientController {
public:
    ClientController(int fd);
    void run();

private:
    void onServerMessage(const std::string& line);
    void handleInput(const std::string& line);

    void handleAuthInput(const std::string& line);
    void handleMenuInput(const std::string& line);
    void handleChatInput(const std::string& line);

    bool isValidUsername(const std::string &username);
    std::string tryDecryptHistoryLine(const std::string &line);

    UIState ui;
    Terminal terminal;
    NetworkManager net;
    
    std::unordered_map<std::string, CryptoSession> sessionStore;
    CryptoHandler crypto;
    FileHandler file;
    VoiceHandler voice;

    std::atomic<bool> running{true};
};
