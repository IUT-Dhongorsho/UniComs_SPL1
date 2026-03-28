#include "VoiceCall.h"
#include <iostream>
#include <chrono>
#include <cstring>

void VoiceCall::start(const std::string &ip, int port)
{
    stop(); // Safely join existing threads before overriding them

    setPeer(ip, port);
    active = true;
    encState = {};
    decState = {};

    captureThread = std::thread(&VoiceCall::captureLoop, this);
    recvThread = std::thread(&VoiceCall::receiveLoop, this);
}

void VoiceCall::stop()
{
    if (!active) return;
    active = false;
    
    if (captureThread.joinable()) captureThread.join();
    if (recvThread.joinable()) recvThread.join();
}

void VoiceCall::setPeer(const std::string &ip, int port)
{
    std::lock_guard<std::mutex> lock(peerMtx);
    peerIp = ip;
    peerPort = port;
}

void VoiceCall::captureLoop()
{
    PaDeviceIndex dev = Pa_GetDefaultInputDevice();
    if (dev == paNoDevice)
    {
        std::cerr << "[voice] No input device\n";
        return;
    }

    PaStreamParameters p{};
    p.device = dev;
    p.channelCount = 1;
    p.sampleFormat = paInt16;
    p.suggestedLatency = Pa_GetDeviceInfo(dev)->defaultLowInputLatency;
    p.hostApiSpecificStreamInfo = nullptr;

    PaStream *stream = nullptr;
    PaError err = Pa_OpenStream(&stream, &p, nullptr,
                                RATE, FRAMES, paClipOff, nullptr, nullptr);
    if (err != paNoError)
    {
        std::cerr << "[voice] Capture open: " << Pa_GetErrorText(err) << "\n";
        return;
    }

    Pa_StartStream(stream);

    std::vector<int16_t> buf(FRAMES);
    while (active)
    {
        err = Pa_ReadStream(stream, buf.data(), FRAMES);
        if (err && err != paInputOverflowed)
            break;

        // Safely extract the current peer data
        std::string targetIp;
        int targetPort;
        {
            std::lock_guard<std::mutex> lock(peerMtx);
            targetIp = peerIp;
            targetPort = peerPort;
        }

        if (targetPort > 0)
        {
            auto encoded = ADPCM::encode(buf, encState);
            udp.send(targetIp, targetPort, encoded);
        }
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
}

void VoiceCall::receiveLoop()
{
    PaDeviceIndex dev = Pa_GetDefaultOutputDevice();
    if (dev == paNoDevice)
    {
        std::cerr << "[voice] No output device\n";
        return;
    }

    PaStreamParameters p{};
    p.device = dev;
    p.channelCount = 1;
    p.sampleFormat = paInt16;
    p.suggestedLatency = Pa_GetDeviceInfo(dev)->defaultLowOutputLatency;
    p.hostApiSpecificStreamInfo = nullptr;

    PaStream *stream = nullptr;
    PaError err = Pa_OpenStream(&stream, nullptr, &p,
                                RATE, FRAMES, paClipOff, nullptr, nullptr);
    if (err != paNoError)
    {
        std::cerr << "[voice] Playback open: " << Pa_GetErrorText(err) << "\n";
        return;
    }

    Pa_StartStream(stream);

    bool firstPacket = true;
    int packetCounter = 0;

    while (active)
    {
        // Drain incoming UDP into queue
        while (true)
        {
            auto pkt = udp.recv();
            if (pkt.empty()) break;

            if (firstPacket)
            {
                std::cout << "\r[voice] <<< Audio stream established! First packet received. >>>\n> " << std::flush;
                firstPacket = false;
            }

            packetCounter++;
            if (packetCounter % 50 == 0)
            {
                std::cout << "." << std::flush;
            }

            auto decoded = ADPCM::decode(pkt, decState);
            std::lock_guard<std::mutex> lock(queueMtx);
            for (int16_t s : decoded)
                playbackQueue.push_back(s);
        }

        std::vector<int16_t> frame(FRAMES, 0);
        {
            std::lock_guard<std::mutex> lock(queueMtx);
            size_t n = std::min(playbackQueue.size(), static_cast<size_t>(FRAMES));
            for (size_t i = 0; i < n; ++i)
            {
                frame[i] = playbackQueue.front();
                playbackQueue.pop_front();
            }
        }

        err = Pa_WriteStream(stream, frame.data(), FRAMES);
        if (err && err != paOutputUnderflowed)
            break;
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
}