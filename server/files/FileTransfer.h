#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <string>

class FileTransfer {
public:
    void sendFile(const std::string& sender,
                  const std::string& receiver,
                  const std::string& filePath);
};

#endif
