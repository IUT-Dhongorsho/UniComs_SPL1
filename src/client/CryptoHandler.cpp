#include "CryptoHandler.hpp"
#include "../utils/utils.h"
#include "base64.h"
#include <iostream>

CryptoHandler::CryptoHandler(int fd, std::unordered_map<std::string, CryptoSession>& sessions, std::string& username)
    : fd(fd), sessionStore(sessions), myUsername(username) {}

bool CryptoHandler::handleDHInit(const std::string& line) {
    if (line.rfind("DH_INIT ", 0) != 0) return false;

    auto p = splitMessage(line, ' ', 4);
    if (p.size() == 4) {
        const std::string &peer = p[1];
        long long theirPub  = std::stoll(p[2]);
        auto theirNonce     = base64Decode(p[3]);

        std::lock_guard<std::mutex> lock(mtx);
        
        // Handle collision: if both initiated, lexicographically smaller username wins the original initiative.
        // If we are "larger", we yield and process their INIT as a replacement for ours.
        if (sessionStore.count(peer) && !sessionStore[peer].ready) {
            if (myUsername < peer) {
                // We are the winner. Ignore their INIT; they will receive our INIT and process it.
                return true; 
            }
        }

        CryptoSession s;
        s.init();
        s.deriveKey(theirPub, theirNonce);
        s.save(peer);
        sessionStore[peer] = s;

        sendLine(fd, "DH_REPLY " + peer + " " +
                        std::to_string(s.pubKey) + " " +
                        base64Encode(s.localNonce));
        return true;
    }
    return false;
}

bool CryptoHandler::handleDHReply(const std::string& line) {
    if (line.rfind("DH_REPLY ", 0) != 0) return false;

    auto p = splitMessage(line, ' ', 4);
    if (p.size() == 4) {
        const std::string &peer = p[1];
        long long theirPub  = std::stoll(p[2]);
        auto theirNonce     = base64Decode(p[3]);

        std::lock_guard<std::mutex> lock(mtx);
        if (sessionStore.count(peer)) {
            sessionStore[peer].deriveKey(theirPub, theirNonce);
            sessionStore[peer].save(peer);
            return true;
        }
    }
    return false;
}

void CryptoHandler::initiateSession(const std::string& peer) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!sessionStore.count(peer) || !sessionStore[peer].ready) {
        CryptoSession s;
        if (s.load(peer)) {
            sessionStore[peer] = s;
            return;
        }
    }

    if (!sessionStore.count(peer) || !sessionStore[peer].ready) {
        CryptoSession s;
        s.init();
        sessionStore[peer] = s;
        sendLine(fd, "DH_INIT " + peer + " " +
                        std::to_string(s.pubKey) + " " + base64Encode(s.localNonce));
    }
}

bool CryptoHandler::isSessionReady(const std::string& peer) {
    std::lock_guard<std::mutex> lock(mtx);
    return sessionStore.count(peer) && sessionStore[peer].ready;
}

std::string CryptoHandler::encrypt(const std::string& peer, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    if (sessionStore.count(peer) && sessionStore[peer].ready) {
        return sessionStore[peer].encryptMsg(msg);
    }
    return msg;
}

std::string CryptoHandler::decrypt(const std::string& peer, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    if (sessionStore.count(peer) && sessionStore[peer].ready) {
        try {
            return sessionStore[peer].decryptMsg(msg);
        } catch (...) {
            return msg;
        }
    }
    return msg;
}
