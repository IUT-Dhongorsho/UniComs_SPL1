#pragma once
#include <cstdint>

long long modexp(long long base, long long exp, long long mod);

long long genPrivKey();
long long genPubKey(long long priv_key);
long long computeSecret(long long priv_key, long long other_pub_key);