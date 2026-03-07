#pragma once
#include <ncurses.h>
#include <string>
#include <map>
#include <chrono>

struct TransferInfo {
    std::string filename;
    std::string peer;
    size_t totalSize;
    size_t transferred;
    bool isUpload;
    bool isComplete;
    bool isFailed;
    std::chrono::steady_clock::time_point startTime;
};

class FileTransferUI {
private:
    WINDOW* window;
    std::map<std::string, TransferInfo> transfers;
    bool isVisible;
    
public:
    FileTransferUI();
    
    void setWindow(WINDOW* win) { window = win; }
    
    // Transfer management
    void startTransfer(const std::string& id, const std::string& filename, 
                      const std::string& peer, size_t size, bool isUpload);
    void updateProgress(const std::string& id, size_t transferred);
    void completeTransfer(const std::string& id, bool success = true);
    void failTransfer(const std::string& id, const std::string& reason = "");
    
    // UI methods
    void show();
    void hide();
    void render();
    
    // Calculate speed
    double getSpeed(const TransferInfo& info);
    std::string getETA(const TransferInfo& info);
    std::string formatSize(size_t bytes);
    
private:
    void drawTransfer(int y, int x, const TransferInfo& info, int width);
};