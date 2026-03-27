#include "Server.h"

// DM <username> <message...>
void cmdDm(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 3)
    {
        sendLine(fd, "ERR Usage: DM <username> <message>");
        return;
    }

    const std::string &toUsername = args[1];
    const std::string &content = args[2];

    std::lock_guard<std::mutex> lock(state.mtx);

    const auto &fromSession = state.sessions[fd];

    auto toUser = state.db.query<User>("username", toUsername);
    if (!toUser)
    {
        sendLine(fd, "ERR User not found: " + toUsername);
        return;
    }

    if (toUser->id == fromSession.userId)
    {
        sendLine(fd, "ERR Cannot DM yourself");
        return;
    }

    // Persist message
    Message msg{generateId(), "dm", fromSession.userId, toUser->id, "", content, currentTimestamp()};
    state.db.insert(msg);

    // Deliver to recipient if online
    for (const auto &[ofd, sess] : state.sessions)
        if (sess.loggedIn && sess.userId == toUser->id)
        {
            std::cout << "[server] DM from " << fromSession.username
                      << " to " << toUsername
                      << " | encrypted: " << content << "\n";
            sendLine(ofd, "MSG_FROM " + fromSession.username + " " + content);
            break;
        }

    sendLine(fd, "OK");
}

// DH_INIT <to_user> <pubkey> <nonce_b64>
// DH_REPLY <to_user> <pubkey> <nonce_b64>
static void cmdDhRelay(int fd, const std::vector<std::string> &args,
                       ServerState &state, const std::string &msgType)
{
    if (args.size() < 4)
    {
        sendLine(fd, "ERR Usage: " + msgType + " <username> <pubkey> <nonce>");
        return;
    }

    const std::string &toUsername = args[1];

    std::lock_guard<std::mutex> lock(state.mtx);
    const auto &fromSession = state.sessions[fd];

    // Find the target user's socket
    for (const auto &[ofd, sess] : state.sessions)
    {
        if (sess.loggedIn && sess.username == toUsername)
        {
            // Forward with sender's username substituted in:
            // "DH_INIT <from_user> <pubkey> <nonce_b64>"
            sendLine(ofd, msgType + " " + fromSession.username + " " +
                              args[2] + " " + args[3]);
            return;
        }
    }

    sendLine(fd, "ERR User not online: " + toUsername);
}

void cmdInitDH(int fd, const std::vector<std::string> &args, ServerState &state)
{
    cmdDhRelay(fd, args, state, "DH_INIT");
}

void cmdReplyDH(int fd, const std::vector<std::string> &args, ServerState &state)
{
    cmdDhRelay(fd, args, state, "DH_REPLY");
}