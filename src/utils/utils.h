#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>

inline void memorySet(void *ptr, int value, size_t size)
{
    memset(ptr, value, size);
}

inline uint16_t hostToNetShort(int port)
{
    return htons(static_cast<uint16_t>(port));
}

// Generate a simple unique ID (timestamp + random suffix)
std::string generateId();

// Get current timestamp as string
std::string currentTimestamp();

// Split a string by a delimiter, returning at most maxParts parts
std::vector<std::string> splitMessage(const std::string &msg, char delim, int maxParts = -1);

// Send a full line to a socket
bool sendLine(int fd, const std::string &msg);

// Receive a full line from a socket (blocking)
std::string recvLine(int fd);