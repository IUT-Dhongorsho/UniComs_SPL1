#include "client.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include "../voice/voice_call.hpp"

static VoiceCall voiceCall;

static const std::string B64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64Encode(const std::vector<uint8_t> &data)
{
    std::string out;
    int val = 0, valb = -6;
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(B64_CHARS[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(B64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

static std::vector<uint8_t> base64Decode(const std::string &s)
{
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T[(uint8_t)B64_CHARS[i]] = i;
    std::vector<uint8_t> out;
    int val = 0, valb = -8;
    for (uint8_t c : s) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return out;
}

static std::string pendingFilePath;
static std::string incomingFilename;
static std::string incomingSender;
static size_t incomingFilesize = 0;
static std::vector<uint8_t> incomingFileData;

static void receiveLoop(int fd, std::atomic<bool> &running)
{
    while (running) {
        std::string line = recvLine(fd);
        if (line.empty()) {
            std::cout << "\n[disconnected from server]\n";
            running = false;
            break;
        }

        if (line.rfind("FILE_ACCEPTED", 0) == 0) {
            if (pendingFilePath.empty()) {
                std::cout << "\r[file] No pending file to send\n> " << std::flush;
                continue;
            }
            std::ifstream file(pendingFilePath, std::ios::binary);
            if (!file) {
                std::cout << "\r[file] Could not open file\n> " << std::flush;
                pendingFilePath.clear();
                continue;
            }
            std::cout << "\r[file] Sending " << pendingFilePath << "...\n> " << std::flush;
            constexpr size_t CHUNK = 3 * 1024;
            std::vector<uint8_t> buf(CHUNK);
            while (file.read(reinterpret_cast<char *>(buf.data()), CHUNK) || file.gcount() > 0) {
                size_t got = static_cast<size_t>(file.gcount());
                buf.resize(got);
                sendLine(fd, "FILE_DATA " + base64Encode(buf));
                buf.resize(CHUNK);
            }
            sendLine(fd, "FILE_END");
            pendingFilePath.clear();
            std::cout << "\r[file] Transfer complete\n> " << std::flush;
            continue;
        }

        if (line.rfind("FILE_REJECTED", 0) == 0) {
            pendingFilePath.clear();
            std::cout << "\r[file] Transfer rejected\n> " << std::flush;
            continue;
        }

        if (line.rfind("FILE_INCOMING ", 0) == 0) {
            auto parts = splitMessage(line, ' ', 3);
            if (parts.size() >= 3) {
                incomingFilename = parts[1];
                incomingFilesize = std::stoull(parts[2]);
                incomingFileData.clear();
                incomingFileData.reserve(incomingFilesize);
                std::cout << "\r[file] Receiving " << incomingFilename
                          << " (" << incomingFilesize << " bytes)...\n> " << std::flush;
            }
            continue;
        }

        if (line.rfind("FILE_DATA ", 0) == 0) {
            auto chunk = base64Decode(line.substr(10));
            incomingFileData.insert(incomingFileData.end(), chunk.begin(), chunk.end());
            continue;
        }

        if (line.rfind("FILE_END", 0) == 0) {
            if (!incomingFilename.empty()) {
                std::string dir = incomingSender.empty() ? "received" : incomingSender;
                std::filesystem::create_directories(dir);
                std::string savePath = dir + "/" + std::filesystem::path(incomingFilename).filename().string();
                std::ofstream out(savePath, std::ios::binary | std::ios::trunc);
                if (out)
                    out.write(reinterpret_cast<const char *>(incomingFileData.data()),
                              static_cast<std::streamsize>(incomingFileData.size()));
                std::cout << "\r[file] Saved to " << savePath << "\n> " << std::flush;
                incomingFilename.clear();
                incomingFilesize = 0;
                incomingFileData.clear();
                incomingSender.clear();
            }
            continue;
        }

        if (line.rfind("FILE_OFFER ", 0) == 0) {
            auto parts = splitMessage(line, ' ', 4);
            if (parts.size() >= 2)
                incomingSender = parts[1];
            std::cout << "\r" << line << "\n";
            std::cout << "  → Type 'FILE_ACCEPT' or 'FILE_REJECT'\n> " << std::flush;
            continue;
        }

        if (line.rfind("CALL_OFFER ", 0) == 0) {
            std::cout << "\r[voice] Incoming call from " << line.substr(11)
                      << "\n  → Type 'CALL_ACCEPT' or 'CALL_REJECT'\n> " << std::flush;
            continue;
        }

        if (line.rfind("CALL_ACCEPTED ", 0) == 0) {
            auto parts = splitMessage(line, ' ', 3);
            std::string peerIp  = parts[1];
            int         peerPort = std::stoi(parts[2]);
            if (peerPort == 0) {
                // We are the caller — peer port arrives later via CALL_PEER_PORT
                voiceCall.start(peerIp, 0);
                sendLine(fd, "CALL_PORT " + std::to_string(voiceCall.udp.localPort));
            } else {
                // We are the receiver — have everything we need
                voiceCall.start(peerIp, peerPort);
            }
            std::cout << "\r[voice] Call connected\n> " << std::flush;
            continue;
        }

        if (line.rfind("CALL_PEER_PORT ", 0) == 0) {
            auto parts       = splitMessage(line, ' ', 3);
            voiceCall.peerIp   = parts[1];
            voiceCall.peerPort = std::stoi(parts[2]);
            std::cout << "\r[voice] Peer ready\n> " << std::flush;
            continue;
        }

        if (line == "CALL_REJECTED") {
            std::cout << "\r[voice] Call rejected\n> " << std::flush;
            continue;
        }

        if (line == "CALL_ENDED") {
            voiceCall.stop();
            std::cout << "\r[voice] Call ended by peer\n> " << std::flush;
            continue;
        }

        std::cout << "\r" << line << "\n> " << std::flush;
    }
}

void runClient(int fd)
{
    std::atomic<bool> running{true};
    std::thread receiver(receiveLoop, fd, std::ref(running));

    std::cout << "Connected. Type commands or 'quit' to exit.\n> " << std::flush;

    std::string line;
    while (running && std::getline(std::cin, line)) {
        if (line == "quit" || line == "exit")
            break;

        auto parts = splitMessage(line, ' ', 3);
        if (parts.empty())
            continue;

        std::string cmd = parts[0];

        if (cmd == "FILE_SEND" && parts.size() >= 3) {
            const std::string &username = parts[1];
            const std::string &filepath = parts[2];
            if (!std::filesystem::exists(filepath)) {
                std::cerr << "File not found: " << filepath << "\n> " << std::flush;
                continue;
            }
            size_t filesize = std::filesystem::file_size(filepath);
            pendingFilePath = filepath;
            sendLine(fd, "FILE_SEND " + username + " " +
                         std::filesystem::path(filepath).filename().string() + " " +
                         std::to_string(filesize));
        } else if (cmd == "FILE_ACCEPT") {
            sendLine(fd, "FILE_ACCEPT");
        } else if (cmd == "FILE_REJECT") {
            sendLine(fd, "FILE_REJECT");
        } else if (cmd == "CALL_ACCEPT") {
            // Start voice early so we have a UDP port to report
            voiceCall.start("", 0);
            sendLine(fd, "CALL_ACCEPT " + std::to_string(voiceCall.udp.localPort));
        } else if (cmd == "CALL_END") {
            voiceCall.stop();
            sendLine(fd, "CALL_END");
        } else {
            sendLine(fd, line);
        }

        std::cout << "> " << std::flush;
    }

    running = false;
    shutdown(fd, SHUT_RDWR);
    close(fd);
    receiver.join();
}

int main(int argc, char *argv[])
{
    std::string host = "127.0.0.1";
    int port = 8080;
    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);

    int fd = clientConnect(host, port);
    if (fd < 0) {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        return 1;
    }

    runClient(fd);
    return 0;
}