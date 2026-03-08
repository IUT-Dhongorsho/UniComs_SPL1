#include "aes.h"
#include <cstring>

static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7,
    0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf,
    0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5,
    0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e,
    0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed,
    0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef,
    0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff,
    0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d,
    0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee,
    0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5,
    0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e,
    0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e,
    0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55,
    0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f,
    0xb0, 0x54, 0xbb, 0x16};

AES256::AES256(const std::vector<uint8_t> &key,
               const std::vector<uint8_t> &nonce)
{
    this->key = key;
    this->nonce = nonce;
    this->roundKey.resize(240);
    counter = 0;

    keyExpansion();
}

uint8_t AES256::xtime(uint8_t x)
{
    return (x << 1) ^ ((x >> 7) * 0x1b);
}

void AES256::subBytes(uint8_t s[4][4])
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            s[i][j] = sbox[s[i][j]];
}

void AES256::shiftRows(uint8_t s[4][4])
{
    uint8_t temp;

    temp = s[1][0];
    s[1][0] = s[1][1];
    s[1][1] = s[1][2];
    s[1][2] = s[1][3];
    s[1][3] = temp;

    temp = s[2][0];
    s[2][0] = s[2][2];
    s[2][2] = temp;

    temp = s[2][1];
    s[2][1] = s[2][3];
    s[2][3] = temp;

    temp = s[3][3];
    s[3][3] = s[3][2];
    s[3][2] = s[3][1];
    s[3][1] = s[3][0];
    s[3][0] = temp;
}

void AES256::mixColumns(uint8_t s[4][4])
{
    uint8_t Tmp, Tm, t;

    for (int i = 0; i < 4; i++)
    {
        t = s[0][i];
        Tmp = s[0][i] ^ s[1][i] ^ s[2][i] ^ s[3][i];

        Tm = s[0][i] ^ s[1][i];
        Tm = xtime(Tm);
        s[0][i] ^= Tm ^ Tmp;
        Tm = s[1][i] ^ s[2][i];
        Tm = xtime(Tm);
        s[1][i] ^= Tm ^ Tmp;
        Tm = s[2][i] ^ s[3][i];
        Tm = xtime(Tm);
        s[2][i] ^= Tm ^ Tmp;
        Tm = s[3][i] ^ t;
        Tm = xtime(Tm);
        s[3][i] ^= Tm ^ Tmp;
    }
}

void AES256::addRoundKey(uint8_t state[4][4], int round)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[j][i] ^= roundKey[round * 16 + i * 4 + j];
}

void AES256::cipher(const std::vector<uint8_t> &in, std::vector<uint8_t> &out)
{
    uint8_t state[4][4];

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[j][i] = in[i * 4 + j];

    addRoundKey(state, 0);

    for (int round = 1; round < 14; round++)
    {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, round);
    }

    subBytes(state);
    shiftRows(state);
    addRoundKey(state, 14);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            out[i * 4 + j] = state[j][i];
}

std::vector<uint8_t> AES256::encrypt(const std::vector<uint8_t> &plaintext)
{
    std::vector<uint8_t> output(plaintext.size());

    std::vector<uint8_t> counterBlock(16);
    std::vector<uint8_t> keystream(16);

    size_t i = 0;

    while (i < plaintext.size())
    {
        counterBlock = nonce;

        for (int j = 0; j < 8; j++)
            counterBlock[15 - j] ^= (counter >> (j * 8)) & 0xFF;

        counter++;

        cipher(counterBlock, keystream);

        for (int j = 0; j < 16 && i < plaintext.size(); j++, i++)
            output[i] = plaintext[i] ^ keystream[j];
    }

    return output;
}

std::vector<uint8_t> AES256::decrypt(const std::vector<uint8_t> &ciphertext)
{
    return encrypt(ciphertext);
}

void AES256::keyExpansion()
{
    for (int i = 0; i < 32; i++)
        roundKey[i] = key[i];

    int bytesGenerated = 32;
    int rcon = 1;
    uint8_t temp[4];

    while (bytesGenerated < 240)
    {
        for (int i = 0; i < 4; i++)
            temp[i] = roundKey[bytesGenerated - 4 + i];

        if (bytesGenerated % 32 == 0)
        {
            uint8_t k = temp[0];
            temp[0] = sbox[temp[1]] ^ rcon;
            temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]];
            temp[3] = sbox[k];

            rcon = xtime(rcon);
        }
        else if (bytesGenerated % 32 == 16)
        {
            // AES-256 extra SubBytes pass at the 16-byte mark
            for (int i = 0; i < 4; i++)
                temp[i] = sbox[temp[i]];
        }

        for (int i = 0; i < 4; i++)
        {
            roundKey[bytesGenerated] =
                roundKey[bytesGenerated - 32] ^ temp[i];
            bytesGenerated++;
        }
    }
}