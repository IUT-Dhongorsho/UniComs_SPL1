#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include "../core/Client.h"

class CommandParser {
public:
    explicit CommandParser(Client& client);
    void readAndExecute();

private:
    Client& client;

    void handleLogin(const std::string& input);
    void handleSignup(const std::string& input);
    void handleDM(const std::string& input);
};

#endif
