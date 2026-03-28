#pragma once
#include <vector>
#include <cstdint>

class AES256 {
public:
    // key = 32 bytes, iv = 16 bytes (used as the initial counter block)
    AES256(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

    // encrypt and decrypt are identical in CTR mode
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& in);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& in);

private:
    void    expandKey();
    void    encryptBlock(const uint8_t in[16], uint8_t out[16]);
    uint8_t xtime(uint8_t x);

    uint8_t rk[240];   // round keys
    uint8_t iv[16];    // initial counter block
};