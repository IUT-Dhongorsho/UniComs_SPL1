#include "CryptoHandler.h"
#include "../utils/Utils.h"
#include "Base64.h"
#include <iostream>

CryptoHandler::CryptoHandler(int fd, std::unordered_map<std::string, CryptoSession> &sessions, std::string &username)
    : fd(fd), sessionStore(sessions), myUsername(username) {}

bool CryptoHandler::handleDHInit(const std::string &line)
{
    if (line.rfind("DH_INIT ", 0) != 0)
        return false;

    auto p = splitMessage(line, ' ', 4);
    if (p.size() == 4)
    {
        const std::string &peer = p[1];
        long long theirPub = std::stoll(p[2]);
        auto theirNonce = base64Decode(p[3]);

        std::lock_guard<std::mutex> lock(mtx);

        // Get or create the session
        auto it = sessionStore.find(peer);
        if (it == sessionStore.end())
        {
            CryptoSession s;
            s.init();
            sessionStore[peer] = s;
            it = sessionStore.find(peer);
        }

        CryptoSession &s = it->second;

        // If the session is already fully established, check if it needs re-keying
        if (s.ready)
        {
            // If pubKey is 0, the session was loaded from disk. We don't have
            // the original DH parameters to replay. The peer has likely lost 
            // their key, so we must initialize new DH parameters to re-key.
            if (s.pubKey == 0) 
            {
                s.init();
            }
            else 
            {
                // We have transient keys in memory, safely resend the reply
                sendLine(fd, "DH_REPLY " + peer + " " +
                                 std::to_string(s.pubKey) + " " +
                                 base64Encode(s.localNonce));
                return true;
            }
        }

        // Use existing or newly initialized session's private key and local nonce to derive shared secret
        s.deriveKey(theirPub, theirNonce);
        
        // Save using the namespaced format: myUsername_peerUsername.key
        s.save(myUsername, peer);

        // Send our public key and local nonce
        sendLine(fd, "DH_REPLY " + peer + " " +
                         std::to_string(s.pubKey) + " " +
                         base64Encode(s.localNonce));
        return true;
    }
    return false;
}

bool CryptoHandler::handleDHReply(const std::string &line)
{
    if (line.rfind("DH_REPLY ", 0) != 0)
        return false;

    auto p = splitMessage(line, ' ', 4);
    if (p.size() == 4)
    {
        const std::string &peer = p[1];
        long long theirPub = std::stoll(p[2]);
        auto theirNonce = base64Decode(p[3]);

        std::lock_guard<std::mutex> lock(mtx);
        if (sessionStore.count(peer))
        {
            if (sessionStore[peer].ready)
                return true; 
                
            sessionStore[peer].deriveKey(theirPub, theirNonce);
            
            // Save using the namespaced format: myUsername_peerUsername.key
            sessionStore[peer].save(myUsername, peer);
            
            return true;
        }
    }
    return false;
}

void CryptoHandler::initiateSession(const std::string &peer)
{
    std::lock_guard<std::mutex> lock(mtx);

    // Only skip if fully established
    if (sessionStore.count(peer) && sessionStore[peer].ready)
        return;

    // Try loading from disk (covers the restart case)
    if (!sessionStore.count(peer))
    {
        CryptoSession s;
        // Load using the namespaced format: myUsername_peerUsername.key
        if (s.load(myUsername, peer)) 
        {
            sessionStore[peer] = s;
            return;
        }
    }

    // Create new session and fire DH_INIT (even if a previous attempt failed)
    CryptoSession s;
    s.init();
    sessionStore[peer] = s;
    sendLine(fd, "DH_INIT " + peer + " " +
                     std::to_string(s.pubKey) + " " + base64Encode(s.localNonce));
}

bool CryptoHandler::isSessionReady(const std::string &peer)
{
    std::lock_guard<std::mutex> lock(mtx);
    return sessionStore.count(peer) && sessionStore[peer].ready;
}

std::string CryptoHandler::encrypt(const std::string &peer, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (sessionStore.count(peer) && sessionStore[peer].ready)
    {
        return sessionStore[peer].encryptMsg(msg);
    }
    return msg;
}

std::string CryptoHandler::decrypt(const std::string &peer, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (sessionStore.count(peer) && sessionStore[peer].ready)
    {
        try
        {
            return sessionStore[peer].decryptMsg(msg);
        }
        catch (...)
        {
            return msg;
        }
    }
    return msg;
}
