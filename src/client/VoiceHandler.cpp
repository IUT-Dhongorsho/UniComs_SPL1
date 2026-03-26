#include "VoiceHandler.hpp"
#include "../utils/utils.h"
#include <iostream>

VoiceHandler::VoiceHandler(int fd) {}

bool VoiceHandler::handleCallOffer(const std::string& line, std::string& outSender) {
    if (line.rfind("CALL_OFFER ", 0) != 0) return false;
    outSender = line.substr(11);
    return true;
}

bool VoiceHandler::handleCallAccepted(const std::string& line, std::string& outIp, int& outPort) {
    if (line.rfind("CALL_ACCEPTED ", 0) != 0) return false;
    auto parts = splitMessage(line, ' ', 3);
    if (parts.size() >= 3) {
        outIp = parts[1];
        outPort = std::stoi(parts[2]);
        if (outPort != 0) {
            voiceCall.start(outIp, outPort);
        } else {
            voiceCall.peerIp = outIp;
        }
    }
    return true;
}

bool VoiceHandler::handleCallPeerPort(const std::string& line, std::string& outIp, int& outPort) {
    if (line.rfind("CALL_PEER_PORT ", 0) != 0) return false;
    auto parts = splitMessage(line, ' ', 3);
    if (parts.size() >= 3) {
        voiceCall.peerIp = parts[1];
        voiceCall.peerPort = std::stoi(parts[2]);
        outIp = voiceCall.peerIp;
        outPort = voiceCall.peerPort;
    }
    return true;
}

bool VoiceHandler::handleCallRejected(const std::string& line) {
    return line == "CALL_REJECTED";
}

bool VoiceHandler::handleCallEnded(const std::string& line) {
    if (line == "CALL_ENDED") {
        voiceCall.stop();
        return true;
    }
    return false;
}

void VoiceHandler::startCall(const std::string& peerIp, int peerPort) {
    voiceCall.start(peerIp, peerPort);
}

void VoiceHandler::stopCall() {
    voiceCall.stop();
}
