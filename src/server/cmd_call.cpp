#include "server.h"

// CALL <username>
void cmdCall(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2) { sendLine(fd, "ERR Usage: CALL <username>"); return; }

    std::lock_guard<std::mutex> lock(state.mtx);

    auto toUser = state.db.query<User>("username", args[1]);
    if (!toUser) { sendLine(fd, "ERR User not found"); return; }

    int receiverFd = -1;
    for (const auto &[ofd, sess] : state.sessions)
        if (sess.loggedIn && sess.userId == toUser->id)
            { receiverFd = ofd; break; }

    if (receiverFd < 0) { sendLine(fd, "ERR User not online"); return; }

    state.pendingCalls[receiverFd] = fd;
    sendLine(receiverFd, "CALL_OFFER " + state.sessions[fd].username);
    sendLine(fd, "INFO Ringing " + args[1] + "...");
}

// CALL_ACCEPT <myUdpPort>
void cmdCallAccept(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2) { sendLine(fd, "ERR Usage: CALL_ACCEPT <udpPort>"); return; }

    std::lock_guard<std::mutex> lock(state.mtx);

    auto it = state.pendingCalls.find(fd);
    if (it == state.pendingCalls.end()) { sendLine(fd, "ERR No incoming call"); return; }

    int callerFd = it->second;
    state.pendingCalls.erase(it);

    state.activeCalls[fd]       = callerFd;
    state.activeCalls[callerFd] = fd;

    auto getIp = [](int sock) {
        sockaddr_in addr{}; socklen_t len = sizeof(addr);
        getpeername(sock, reinterpret_cast<sockaddr *>(&addr), &len);
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
        return std::string(buf);
    };

    // Tell each side: peer's IP + peer's UDP port
    // Receiver knows their own port; caller will report theirs via CALL_PORT
    sendLine(callerFd, "CALL_ACCEPTED " + getIp(fd)       + " " + args[1]);
    sendLine(fd,       "CALL_ACCEPTED " + getIp(callerFd) + " 0");
}

// CALL_PORT <udpPort>  — caller reports their UDP port after receiving CALL_ACCEPTED
void cmdCallPort(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2) { sendLine(fd, "ERR Usage: CALL_PORT <udpPort>"); return; }

    std::lock_guard<std::mutex> lock(state.mtx);

    auto it = state.activeCalls.find(fd);
    if (it == state.activeCalls.end()) { sendLine(fd, "ERR No active call"); return; }

    auto getIp = [](int sock) {
        sockaddr_in addr{}; socklen_t len = sizeof(addr);
        getpeername(sock, reinterpret_cast<sockaddr *>(&addr), &len);
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
        return std::string(buf);
    };

    sendLine(it->second, "CALL_PEER_PORT " + getIp(fd) + " " + args[1]);
}

// CALL_REJECT
void cmdCallReject(int fd, const std::vector<std::string> &args, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);

    auto it = state.pendingCalls.find(fd);
    if (it == state.pendingCalls.end()) { sendLine(fd, "ERR No incoming call"); return; }

    sendLine(it->second, "CALL_REJECTED");
    state.pendingCalls.erase(it);
}

// CALL_END
void cmdCallEnd(int fd, const std::vector<std::string> &args, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);

    auto it = state.activeCalls.find(fd);
    if (it != state.activeCalls.end()) {
        sendLine(it->second, "CALL_ENDED");
        state.activeCalls.erase(it->second);
        state.activeCalls.erase(fd);
    }
}