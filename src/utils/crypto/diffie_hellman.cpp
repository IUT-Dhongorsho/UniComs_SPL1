#include "diffie_hellman.h"
#include <random>

long long modexp(long long base, long long exp, long long mod)
{
    long long result = 1;
    base = base % mod;

    while (exp > 0)
    {
        if (exp & 1)
            result = (result * base) % mod;

        base = (base * base) % mod;
        exp >>= 1;
    }

    return result;
}

long long genPrivKey()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dist(2, 20);

    return dist(gen);
}

long long genPubKey(long long priv_key)
{
    long long g = 5;
    long long p = 23;

    return modexp(g, priv_key, p);
}

long long computeSecret(long long priv_key, long long other_pub_key)
{
    long long p = 23;

    return modexp(other_pub_key, priv_key, p);
}