#pragma once
#include <string>
#include <vector>
#include <cstdint>

class FileHandler {
public:
    FileHandler(int fd);

    bool handleFileAccepted(const std::string& line);
    bool handleFileRejected(const std::string& line);
    bool handleFileIncoming(const std::string& line);
    bool handleFileData(const std::string& line);
    bool handleFileEnd(const std::string& line, std::string& savedPath);
    bool handleFileOffer(const std::string& line, std::string& outSender, std::string& fname, std::string& fsize);

    void setPendingPath(const std::string& path) { pendingFilePath = path; }
    const std::string& getPendingPath() const { return pendingFilePath; }
    const std::string& getIncomingSender() const { return incomingSender; }
    void setIncomingSender(const std::string& sender) { incomingSender = sender; }

private:
    int fd;
    std::string pendingFilePath;
    std::string incomingFilename;
    std::string incomingSender;
    size_t incomingFilesize = 0;
    std::vector<uint8_t> incomingFileData;
};
