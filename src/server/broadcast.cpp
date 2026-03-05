#include "server.h"

// DM <username> <message...>
void cmdDm(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 3)
    {
        sendLine(fd, "ERR Usage: DM <username> <message>");
        return;
    }

    const std::string &toUsername = args[1];
    const std::string &content    = args[2];

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
            sendLine(ofd, "MSG_FROM " + fromSession.username + " " + content);
            break;
        }

    sendLine(fd, "OK");
}