#include "Utils.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <ctime>
#include "Sha256.h"
#include <fstream>

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

std::string generateSalt()
{
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (!urandom) throw std::runtime_error("Cannot open /dev/urandom");

    unsigned char buf[16];
    urandom.read(reinterpret_cast<char*>(buf), sizeof(buf));

    std::ostringstream ss;
    for (int i = 0; i < 16; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)buf[i];
    return ss.str();
}

std::string hashPassword(const std::string &password)
{
    std::string salt = generateSalt();
    std::string hash = sha256(salt + password);
    for (int i = 0; i < 10000; ++i)
        hash = sha256(hash);
    return salt + ":" + hash;
}

bool verifyPassword(const std::string &password, const std::string &stored)
{
    auto colon = stored.find(':');
    if (colon == std::string::npos) return false;

    std::string salt = stored.substr(0, colon);
    std::string expected = stored.substr(colon + 1);

    std::string hash = sha256(salt + password);
    for (int i = 0; i < 10000; ++i)
        hash = sha256(hash);

    // Constant-time compare
    if (hash.size() != expected.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < hash.size(); ++i)
        diff |= hash[i] ^ expected[i];
    return diff == 0;
}
