#include <cstdint>

uint16_t hostToNetShort(uint16_t x)
{
    return (x << 8) | (x >> 8);
}