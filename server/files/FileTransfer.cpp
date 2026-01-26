#include "FileTransfer.h"
#include "../../common/utils/Logger.h"

void FileTransfer::sendFile(const std::string& sender,
                            const std::string& receiver,
                            const std::string& filePath) {
    Logger::info("File transfer requested from " + sender +
                 " to " + receiver +
                 " for file " + filePath);
}
