#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include "../utils/crypto/DiffieHellman.h"
#include "../utils/Sha256.h"
#include "../utils/crypto/Aes.h"
#include "../utils/crypto/Random.h"
#include "Base64.h"

// Directory where per-peer keys are stored
static const std::string KEY_DIR = "keys/";
struct CryptoSession
{
    long long privKey = 0;
    long long pubKey = 0;
    long long sharedSecret = 0;
    std::vector<uint8_t> localNonce; // Our original random nonce
    std::vector<uint8_t> aesKey;     // 32 bytes
    std::vector<uint8_t> nonce;      // Final XORed nonce (16 bytes)
    bool ready = false;

    void init()
    {
        privKey = genPrivKey();
        pubKey = genPubKey(privKey);
        localNonce = randomBytes(16);
        nonce.resize(16);
    }

    void deriveKey(long long otherPubKey, const std::vector<uint8_t> &theirNonce)
    {
        sharedSecret = computeSecret(privKey, otherPubKey);

        std::string hash = sha256(std::to_string(sharedSecret));
        aesKey.assign(32, 0);
        for (int i = 0; i < 32; i++)
            aesKey[i] = static_cast<uint8_t>(
                std::stoul(hash.substr(i * 2, 2), nullptr, 16));

        nonce.resize(16);
        for (int i = 0; i < 16; i++)
            nonce[i] = localNonce[i] ^ theirNonce[i];

        ready = true;
    }

    // Save aesKey + nonce to ~/.chatapp/keys/<peer>.key
    void save(const std::string &myUsername, const std::string &peer) const
    {
        if (!ready)
            return;
        std::filesystem::create_directories(KEY_DIR);
        std::ofstream f(KEY_DIR + myUsername + "_" + peer + ".key", std::ios::binary | std::ios::trunc);
        if (!f)
            return;
        f.write(reinterpret_cast<const char *>(aesKey.data()), 32);
        f.write(reinterpret_cast<const char *>(nonce.data()), 16);
    }

    // Load aesKey + nonce
    bool load(const std::string &myUsername, const std::string &peer)
    {
        std::ifstream f(KEY_DIR + myUsername + "_" + peer + ".key", std::ios::binary);
        if (!f)
            return false;
        aesKey.resize(32);
        nonce.resize(16);
        f.read(reinterpret_cast<char *>(aesKey.data()), 32);
        f.read(reinterpret_cast<char *>(nonce.data()), 16);
        if (!f)
            return false; // file was too short
        ready = true;
        return true;
    }

    std::string encryptMsg(const std::string &plaintext)
    {
        // generate 8 random bytes → use as the counter start
        auto iv_rand = randomBytes(8);

        // build a 16-byte IV: 8 random bytes + 8 zero bytes
        // the AES CTR counter increments the last 8 bytes, so this gives
        // 2^64 unique counter values per random prefix — plenty for a chat app
        std::vector<uint8_t> iv(16, 0);
        memcpy(iv.data(), iv_rand.data(), 8);

        AES256 aes(aesKey, iv);
        std::vector<uint8_t> pt(plaintext.begin(), plaintext.end());
        auto ct = aes.encrypt(pt);

        // prepend the 8-byte random IV so the receiver can reconstruct it
        std::vector<uint8_t> out;
        out.insert(out.end(), iv_rand.begin(), iv_rand.end());
        out.insert(out.end(), ct.begin(), ct.end());
        return base64Encode(out);
    }

    std::string decryptMsg(const std::string &b64)
    {
        auto raw = base64Decode(b64);
        if (raw.size() < 8)
            return b64; // too short, return as-is

        // reconstruct the same 16-byte IV from the 8-byte prefix
        std::vector<uint8_t> iv(16, 0);
        memcpy(iv.data(), raw.data(), 8);

        AES256 aes(aesKey, iv);
        std::vector<uint8_t> ct(raw.begin() + 8, raw.end());
        auto pt = aes.decrypt(ct);
        return std::string(pt.begin(), pt.end());
    }
};