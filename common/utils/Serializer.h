#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <vector>
#include "../protocol/Packet.h"

class Serializer {
public:
    static std::vector<char> serialize(const Packet& packet);
    static Packet deserialize(const std::vector<char>& buffer);
};

#endif
