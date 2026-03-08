#pragma once
#include <vector>
#include <string>
#include <cstdint>

static const std::string B64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string base64Encode(const std::vector<uint8_t> &data)
{
    std::string out;
    int val = 0, valb = -6;
    for (uint8_t c : data)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back(B64_CHARS[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(B64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

inline std::vector<uint8_t> base64Decode(const std::string &s)
{
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T[(uint8_t)B64_CHARS[i]] = i;

    std::vector<uint8_t> out;
    int val = 0, valb = -8;
    for (uint8_t c : s)
    {
        if (T[c] == -1)
            break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0)
        {
            out.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return out;
}