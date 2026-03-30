#pragma once
#include <vector>
#include <cstdint>

// random byte generator
std::vector<uint8_t> randomBytes(size_t n);

// random 64-bit integer (for Diffie-Hellman keys)
long long randomInt(long long min, long long max);