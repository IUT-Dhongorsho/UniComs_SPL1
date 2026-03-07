#include "random.h"
#include <random>

std::vector<uint8_t> randomBytes(size_t n)
{
    std::vector<uint8_t> bytes(n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    for (size_t i = 0; i < n; i++)
        bytes[i] = static_cast<uint8_t>(dist(gen));

    return bytes;
}

long long randomInt(long long min, long long max)
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<long long> dist(min, max);

    return dist(gen);
}