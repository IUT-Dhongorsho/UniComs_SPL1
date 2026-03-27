#include "CryptoHandler.h"
#include "../utils/Utils.h"
#include "Base64.h"
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

        // Get or create the session
        auto it = sessionStore.find(peer);
        if (it == sessionStore.end()) {
            CryptoSession s;
            s.init();
            sessionStore[peer] = s;
            it = sessionStore.find(peer);
        }

        CryptoSession &s = it->second;

        // If the session is already fully established, ignore this duplicate INIT
        if (s.ready) {
            return true;
        }

        // Use existing session's private key and local nonce to derive shared secret
        s.deriveKey(theirPub, theirNonce);
        s.save(peer);

        // Send our public key and local nonce (these come from the session, which was either
        // created in this call or from a previous initiateSession call)
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
    // If a session already exists (in progress or established), do nothing.
    if (sessionStore.count(peer)) {
        return;
    }

    // Try to load a saved session from disk
    CryptoSession s;
    if (s.load(peer)) {
        sessionStore[peer] = s;
        return;
    }

    // No saved session; create a new one and initiate DH
    s.init();
    sessionStore[peer] = s;
    sendLine(fd, "DH_INIT " + peer + " " +
                    std::to_string(s.pubKey) + " " + base64Encode(s.localNonce));
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
