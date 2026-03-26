#pragma once
#include <string>
#include "../voice/voice_call.hpp"

class VoiceHandler {
public:
    VoiceHandler(int fd);

    bool handleCallOffer(const std::string& line, std::string& outSender);
    bool handleCallAccepted(const std::string& line, std::string& outIp, int& outPort);
    bool handleCallPeerPort(const std::string& line, std::string& outIp, int& outPort);
    bool handleCallRejected(const std::string& line);
    bool handleCallEnded(const std::string& line);

    void startCall(const std::string& peerIp, int peerPort);
    void stopCall();
    int getLocalPort() const { return voiceCall.udp.localPort; }

private:
    VoiceCall voiceCall;
};
