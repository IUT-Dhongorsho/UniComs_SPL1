#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include "../utils/crypto/diffie_hellman.h"
#include "../utils/sha256.h"
#include "../utils/crypto/aes.h"
#include "../utils/crypto/random.h"
#include "base64.h"

// Holds per-peer crypto state for one client-to-client session
struct CryptoSession
{
    long long privKey = 0;
    long long pubKey = 0;
    long long sharedSecret = 0;
    std::vector<uint8_t> aesKey; // 32 bytes derived via SHA-256
    std::vector<uint8_t> nonce;  // 16 bytes, XOR of both sides' nonces
    bool ready = false;

    // Step 1 — generate our keypair and nonce, call before sending DH_INIT
    void init()
    {
        privKey = genPrivKey();
        pubKey = genPubKey(privKey);
        nonce = randomBytes(16);
    }

    // Step 2 — call once we have the peer's public key and their raw nonce.
    // Both sides XOR their own nonce with the peer's nonce, so both arrive
    // at the same final nonce without any convention about "whose nonce wins".
    void deriveKey(long long otherPubKey, const std::vector<uint8_t> &theirNonce)
    {
        sharedSecret = computeSecret(privKey, otherPubKey);

        // Derive 32-byte AES key from shared secret via SHA-256
        std::string hash = sha256(std::to_string(sharedSecret));
        aesKey.resize(32);
        for (int i = 0; i < 32; i++)
            aesKey[i] = static_cast<uint8_t>(
                std::stoul(hash.substr(i * 2, 2), nullptr, 16));

        // XOR both nonces — deterministic, no "first client" convention needed
        for (int i = 0; i < 16; i++)
            nonce[i] ^= theirNonce[i];

        ready = true;
    }

    // Encrypt a plaintext string → base64 ciphertext string
    std::string encryptMsg(const std::string &plaintext)
    {
        AES256 aes(aesKey, nonce);
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        return base64Encode(aes.encrypt(pt));
    }

    // Decrypt a base64 ciphertext string → plaintext string
    std::string decryptMsg(const std::string &b64ciphertext)
    {
        AES256 aes(aesKey, nonce);
        auto ct = base64Decode(b64ciphertext);
        auto pt = aes.decrypt(ct);
        return std::string(pt.begin(), pt.end());
    }
};

// Per-peer session store — keyed by peer username.
// Declared here, defined once in client.cpp.
extern std::unordered_map<std::string, CryptoSession> sessionStore;