#ifndef PACKET_TYPES_H
#define PACKET_TYPES_H

#include <cstdint>

// Every packet sent over the network has one of these types
enum class PacketType : uint16_t {
    LOGIN = 1,
    SIGNUP,
    LOGOUT,

    DIRECT_MESSAGE,
    GROUP_MESSAGE,

    FILE_META,
    FILE_CHUNK,

    ACK,
    ERROR
};

#endif
