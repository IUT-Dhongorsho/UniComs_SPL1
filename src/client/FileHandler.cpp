#include "FileHandler.hpp"
#include "../utils/utils.h"
#include "base64.h"
#include <iostream>
#include <fstream>
#include <filesystem>

FileHandler::FileHandler(int fd) : fd(fd) {}

bool FileHandler::handleFileAccepted(const std::string& line) {
    if (line.rfind("FILE_ACCEPTED", 0) != 0) return false;
    if (pendingFilePath.empty()) return true;

    std::ifstream file(pendingFilePath, std::ios::binary);
    if (!file) {
        pendingFilePath.clear();
        return true;
    }

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
    return true;
}

bool FileHandler::handleFileRejected(const std::string& line) {
    if (line.rfind("FILE_REJECTED", 0) != 0) return false;
    pendingFilePath.clear();
    return true;
}

bool FileHandler::handleFileIncoming(const std::string& line) {
    if (line.rfind("FILE_INCOMING ", 0) != 0) return false;
    auto parts = splitMessage(line, ' ', 3);
    if (parts.size() >= 3) {
        incomingFilename = parts[1];
        incomingFilesize = std::stoull(parts[2]);
        incomingFileData.clear();
        incomingFileData.reserve(incomingFilesize);
    }
    return true;
}

bool FileHandler::handleFileData(const std::string& line) {
    if (line.rfind("FILE_DATA ", 0) != 0) return false;
    auto chunk = base64Decode(line.substr(10));
    incomingFileData.insert(incomingFileData.end(), chunk.begin(), chunk.end());
    return true;
}

bool FileHandler::handleFileEnd(const std::string& line, std::string& savedPath) {
    if (line.rfind("FILE_END", 0) != 0) return false;
    if (!incomingFilename.empty()) {
        std::string dir = incomingSender.empty() ? "received" : incomingSender;
        std::filesystem::create_directories(dir);
        std::string savePath = dir + "/" +
            std::filesystem::path(incomingFilename).filename().string();
        std::ofstream out(savePath, std::ios::binary | std::ios::trunc);
        if (out) {
            out.write(reinterpret_cast<const char *>(incomingFileData.data()),
                      static_cast<std::streamsize>(incomingFileData.size()));
            savedPath = savePath;
        }
        incomingFilename.clear();
        incomingFilesize = 0;
        incomingFileData.clear();
        incomingSender.clear();
    }
    return true;
}

bool FileHandler::handleFileOffer(const std::string& line, std::string& outSender, std::string& fname, std::string& fsize) {
    if (line.rfind("FILE_OFFER ", 0) != 0) return false;
    auto parts = splitMessage(line, ' ', 4);
    if (parts.size() >= 4) {
        incomingSender = parts[1];
        outSender = incomingSender;
        fname = parts[2];
        fsize = parts[3];
    }
    return true;
}
