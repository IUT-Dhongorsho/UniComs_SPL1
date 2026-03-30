#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <portaudio.h>
#include "Adpcm.h"
#include "UdpSocket.h"

class VoiceCall {
public:
    VoiceCall() { Pa_Initialize(); }
    ~VoiceCall() { 
        stop(); 
        Pa_Terminate(); 
    }

    std::atomic<bool> active{false};
    UDPSocket         udp;

    void start(const std::string &ip, int port);
    void stop();
    
    // Safely update peer config from the network thread
    void setPeer(const std::string &ip, int port);

private:
    std::string peerIp;
    int         peerPort = 0;
    std::mutex  peerMtx;

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