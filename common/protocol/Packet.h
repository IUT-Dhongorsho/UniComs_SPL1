#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <vector>
#include <cstdint>
#include "PacketTypes.h"

struct Packet {
    uint32_t magic;            // Used to validate packet
    PacketType type;           // Message type
    std::string sender;        // Username of sender
    std::string recipient;     // Username or group
    uint32_t payload_length;   // Size of data
    uint64_t timestamp;        // Unix timestamp
    std::vector<char> payload; // Actual data
};

#endif
