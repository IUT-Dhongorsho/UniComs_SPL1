#include "Serializer.h"
#include <cstring>
#include <ctime>

static const uint32_t MAGIC_NUMBER = 0xAABBCCDD;

std::vector<char> Serializer::serialize(const Packet& packet) {
    std::vector<char> buffer;

    auto append = [&](const void* data, size_t size) {
        const char* bytes = static_cast<const char*>(data);
        buffer.insert(buffer.end(), bytes, bytes + size);
    };

    append(&MAGIC_NUMBER, sizeof(MAGIC_NUMBER));

    uint16_t type = static_cast<uint16_t>(packet.type);
    append(&type, sizeof(type));

    uint32_t sender_len = packet.sender.size();
    append(&sender_len, sizeof(sender_len));
    append(packet.sender.data(), sender_len);

    uint32_t recipient_len = packet.recipient.size();
    append(&recipient_len, sizeof(recipient_len));
    append(packet.recipient.data(), recipient_len);

    append(&packet.payload_length, sizeof(packet.payload_length));
    append(&packet.timestamp, sizeof(packet.timestamp));

    append(packet.payload.data(), packet.payload.size());

    return buffer;
}

Packet Serializer::deserialize(const std::vector<char>& buffer) {
    Packet packet;
    size_t offset = 0;

    auto read = [&](void* dest, size_t size) {
        std::memcpy(dest, buffer.data() + offset, size);
        offset += size;
    };

    read(&packet.magic, sizeof(packet.magic));

    uint16_t type;
    read(&type, sizeof(type));
    packet.type = static_cast<PacketType>(type);

    uint32_t sender_len;
    read(&sender_len, sizeof(sender_len));
    packet.sender.assign(buffer.data() + offset, sender_len);
    offset += sender_len;

    uint32_t recipient_len;
    read(&recipient_len, sizeof(recipient_len));
    packet.recipient.assign(buffer.data() + offset, recipient_len);
    offset += recipient_len;

    read(&packet.payload_length, sizeof(packet.payload_length));
    read(&packet.timestamp, sizeof(packet.timestamp));

    packet.payload.assign(
        buffer.data() + offset,
        buffer.data() + offset + packet.payload_length
    );

    return packet;
}
