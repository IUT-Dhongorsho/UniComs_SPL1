#include "utils.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <ctime>

std::string generateId()
{
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    return std::to_string(now) + std::to_string(rand() % 10000);
}

std::string currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::vector<std::string> splitMessage(const std::string &msg, char delim, int maxParts)
{
    std::vector<std::string> parts;
    std::stringstream ss(msg);
    std::string token;
    int count = 0;

    while (std::getline(ss, token, delim))
    {
        count++;
        if (maxParts > 0 && count == maxParts)
        {
            // Rest of the string goes into last token
            std::string rest;
            if (std::getline(ss, rest))
                token += delim + rest;
            parts.push_back(token);
            break;
        }
        parts.push_back(token);
    }
    return parts;
}

bool sendLine(int fd, const std::string &msg)
{
    std::string line = msg + "\n";
    size_t sent = 0;
    while (sent < line.size())
    {
        ssize_t n = send(fd, line.c_str() + sent, line.size() - sent, 0);
        if (n <= 0)
            return false;
        sent += n;
    }
    return true;
}

std::string recvLine(int fd)
{
    std::string result;
    char c;
    while (true)
    {
        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0)
            return "";
        if (c == '\n')
            break;
        result += c;
    }
    // Strip trailing \r if present
    if (!result.empty() && result.back() == '\r')
        result.pop_back();
    return result;
}