#pragma once
#include <vector>
#include <cstdint>

class AES256
{
public:
    AES256(const std::vector<uint8_t> &key,
           const std::vector<uint8_t> &nonce);

    std::vector<uint8_t> encrypt(const std::vector<uint8_t> &plaintext);
    std::vector<uint8_t> decrypt(const std::vector<uint8_t> &ciphertext);

private:
    void keyExpansion();
    void cipher(const std::vector<uint8_t> &in, std::vector<uint8_t> &out);

    void subBytes(uint8_t state[4][4]);
    void shiftRows(uint8_t state[4][4]);
    void mixColumns(uint8_t state[4][4]);
    void addRoundKey(uint8_t state[4][4], int round);

    uint8_t xtime(uint8_t x);

private:
    std::vector<uint8_t> key;      // 32 bytes
    std::vector<uint8_t> roundKey; // 240 bytes

    std::vector<uint8_t> nonce; // 16 bytes
    uint64_t counter;
};