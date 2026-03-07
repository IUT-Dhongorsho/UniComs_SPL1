#include "FileTransferUI.hpp"
#include "../Colors.hpp"
#include "../Animations.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

FileTransferUI::FileTransferUI() : window(nullptr), isVisible(false) {}

void FileTransferUI::startTransfer(const std::string& id, const std::string& filename,
                                  const std::string& peer, size_t size, bool isUpload) {
    TransferInfo info;
    info.filename = filename;
    info.peer = peer;
    info.totalSize = size;
    info.transferred = 0;
    info.isUpload = isUpload;
    info.isComplete = false;
    info.isFailed = false;
    info.startTime = std::chrono::steady_clock::now();
    
    transfers[id] = info;
    show();
}

void FileTransferUI::updateProgress(const std::string& id, size_t transferred) {
    auto it = transfers.find(id);
    if (it != transfers.end()) {
        it->second.transferred = transferred;
    }
}

void FileTransferUI::completeTransfer(const std::string& id, bool success) {
    auto it = transfers.find(id);
    if (it != transfers.end()) {
        it->second.isComplete = success;
        it->second.isFailed = !success;
    }
}

void FileTransferUI::failTransfer(const std::string& id, const std::string& reason) {
    auto it = transfers.find(id);
    if (it != transfers.end()) {
        it->second.isFailed = true;
    }
}

double FileTransferUI::getSpeed(const TransferInfo& info) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - info.startTime).count();
    
    if (elapsed == 0) return 0;
    return static_cast<double>(info.transferred) / elapsed;
}

std::string FileTransferUI::getETA(const TransferInfo& info) {
    double speed = getSpeed(info);
    if (speed == 0) return "∞";
    
    size_t remaining = info.totalSize - info.transferred;
    int seconds = static_cast<int>(remaining / speed);
    
    if (seconds < 60) {
        return std::to_string(seconds) + "s";
    } else if (seconds < 3600) {
        return std::to_string(seconds / 60) + "m" + 
               std::to_string(seconds % 60) + "s";
    } else {
        return std::to_string(seconds / 3600) + "h" + 
               std::to_string((seconds % 3600) / 60) + "m";
    }
}

std::string FileTransferUI::formatSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double size = bytes;
    
    while (size >= 1024 && unit < 3) {
        size /= 1024;
        unit++;
    }
    
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << size << " " << units[unit];
    return ss.str();
}

void FileTransferUI::show() {
    isVisible = true;
}

void FileTransferUI::hide() {
    isVisible = false;
}

void FileTransferUI::render() {
    if (!isVisible || !window) return;
    
    werase(window);
    
    int height = getmaxy(window);
    int width = getmaxx(window);
    
    // Draw border and title
    box(window, 0, 0);
    wattron(window, COLOR_PAIR(Colors::HEADER) | A_BOLD);
    mvwprintw(window, 0, 2, " FILE TRANSFERS ");
    wattroff(window, COLOR_PAIR(Colors::HEADER) | A_BOLD);
    
    if (transfers.empty()) {
        wattron(window, COLOR_PAIR(Colors::TIMESTAMP));
        mvwprintw(window, height/2, (width - 20)/2, "No active transfers");
        wattroff(window, COLOR_PAIR(Colors::TIMESTAMP));
        wrefresh(window);
        return;
    }
    
    int y = 2;
    for (const auto& [id, info] : transfers) {
        if (info.isComplete || info.isFailed) continue;
        if (y >= height - 3) break;
        
        drawTransfer(y, 2, info, width - 4);
        y += 4;
    }
    
    wrefresh(window);
}

void FileTransferUI::drawTransfer(int y, int x, const TransferInfo& info, int width) {
    // Direction indicator
    if (info.isUpload) {
        wattron(window, COLOR_PAIR(Colors::WARNING));
        mvwprintw(window, y, x, "↑");
        wattroff(window, COLOR_PAIR(Colors::WARNING));
    } else {
        wattron(window, COLOR_PAIR(Colors::SUCCESS));
        mvwprintw(window, y, x, "↓");
        wattroff(window, COLOR_PAIR(Colors::SUCCESS));
    }
    
    // Filename and peer
    wattron(window, A_BOLD);
    std::string filename = info.filename.substr(0, 20);
    mvwprintw(window, y, x + 2, "%s", filename.c_str());
    wattroff(window, A_BOLD);
    
    mvwprintw(window, y, x + 25, "→ %s", info.peer.c_str());
    
    // Progress
    int progress = static_cast<int>(info.transferred * 100.0 / info.totalSize);
    int barWidth = width - 40;
    int filled = (progress * barWidth) / 100;
    
    mvwprintw(window, y + 1, x, "[");
    for(int i = 0; i < barWidth; i++) {
        if(i < filled) {
            if(progress < 30) wattron(window, COLOR_PAIR(Colors::WARNING));
            else if(progress < 70) wattron(window, COLOR_PAIR(Colors::HIGHLIGHT));
            else wattron(window, COLOR_PAIR(Colors::SUCCESS));
            
            mvwprintw(window, y + 1, x + i + 1, "█");
        } else {
            wattron(window, COLOR_PAIR(Colors::OFFLINE));
            mvwprintw(window, y + 1, x + i + 1, "░");
        }
    }
    mvwprintw(window, y + 1, x + barWidth + 1, "] %d%%", progress);
    
    // Size and speed
    std::string transferred = formatSize(info.transferred);
    std::string total = formatSize(info.totalSize);
    double speed = getSpeed(info);
    std::string eta = getETA(info);
    
    mvwprintw(window, y + 2, x, "%s / %s | Speed: %s/s | ETA: %s",
              transferred.c_str(), total.c_str(),
              formatSize(static_cast<size_t>(speed)).c_str(),
              eta.c_str());
}