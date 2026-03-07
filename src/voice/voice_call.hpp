#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <portaudio.h>
#include "adpcm.hpp"
#include "udp_socket.hpp"

class VoiceCall {
public:
    VoiceCall() { Pa_Initialize(); }
    ~VoiceCall() { 
        stop(); 
        Pa_Terminate(); 
    }

    std::string       peerIp;
    std::atomic<int>  peerPort{0};
    std::atomic<bool> active{false};
    UDPSocket         udp;

    void start(const std::string &ip, int port);
    void stop();

private:
    std::thread  captureThread;
    std::thread  recvThread;
    ADPCM::State encState;
    ADPCM::State decState;

    std::mutex          queueMtx;
    std::deque<int16_t> playbackQueue;

    static constexpr int RATE   = 16000;
    static constexpr int FRAMES = 320;

    void captureLoop();
    void receiveLoop();
};