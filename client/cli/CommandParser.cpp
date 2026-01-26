#include "CommandParser.h"
#include <iostream>
#include <sstream>
#include "../../common/utils/Serializer.h"
#include "../../common/protocol/Packet.h"

CommandParser::CommandParser(Client& client) : client(client) {}

void CommandParser::readAndExecute() {
    std::string input;
    std::getline(std::cin, input);
    std::cout << "[DEBUG] You typed: " << input << std::endl;

    if (input.rfind("/login", 0) == 0)
        handleLogin(input);
    else if (input.rfind("/signup", 0) == 0)
        handleSignup(input);
    else if (input.rfind("/dm", 0) == 0)
        handleDM(input);
}

void CommandParser::handleLogin(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd, user, pass;
    iss >> cmd >> user >> pass;

    Packet p;
    p.type = PacketType::LOGIN;
    p.sender = user;
    p.payload.assign(pass.begin(), pass.end());
    p.payload_length = p.payload.size();

    client.getSocket().sendBytes(Serializer::serialize(p));
    std::cout << "[INFO] Login request sent\n";
}

void CommandParser::handleSignup(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd, user, pass;
    iss >> cmd >> user >> pass;

    Packet p;
    p.type = PacketType::SIGNUP;
    p.sender = user;
    p.payload.assign(pass.begin(), pass.end());
    p.payload_length = p.payload.size();

    client.getSocket().sendBytes(Serializer::serialize(p));
    std::cout << "[INFO] Signup request sent\n";
}

void CommandParser::handleDM(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd, to, msg;
    iss >> cmd >> to;
    std::getline(iss, msg);

    Packet p;
    p.type = PacketType::DIRECT_MESSAGE;
    p.recipient = to;
    p.payload.assign(msg.begin(), msg.end());
    p.payload_length = p.payload.size();
    p.sender = "tabib"; // TEMPORARY
    client.getSocket().sendBytes(Serializer::serialize(p));
    std::cout << "[INFO] DM request sent\n";
}
