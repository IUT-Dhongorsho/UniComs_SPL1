#include "Server.h"

// SIGNUP <username> <password>
void cmdSignup(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 3)
    {
        sendLine(fd, "ERR Usage: SIGNUP <username> <password>");
        return;
    }

    const std::string &username = args[1];
    const std::string &password = args[2];

    std::lock_guard<std::mutex> lock(state.mtx);

    if (state.db.query<User>("username", username))
    {
        sendLine(fd, "ERR Username already taken");
        return;
    }

    User user{generateId(), username, hashPassword(password)};
    try
    {
        state.db.insert(user);
        sendLine(fd, "OK Signup successful. You can now LOGIN.");
    }
    catch (const std::exception &e)
    {
        sendLine(fd, std::string("ERR ") + e.what());
    }
}

// LOGIN <username> <password>
void cmdLogin(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 3)
    {
        sendLine(fd, "ERR Usage: LOGIN <username> <password>");
        return;
    }

    const std::string &username = args[1];
    const std::string &password = args[2];

    std::lock_guard<std::mutex> lock(state.mtx);

    if (state.sessions[fd].loggedIn)
    {
        sendLine(fd, "ERR Already logged in");
        return;
    }

    auto user = state.db.query<User>("username", username);
    if (!user || !verifyPassword(password, user->password))
    {
        sendLine(fd, "ERR Invalid username or password");
        return;
    }

    // Check not already logged in on another connection
    for (const auto &[ofd, sess] : state.sessions)
        if (sess.loggedIn && sess.userId == user->id)
        {
            sendLine(fd, "ERR Already logged in on another connection");
            return;
        }

    state.sessions[fd].loggedIn = true;
    state.sessions[fd].userId   = user->id;
    state.sessions[fd].username = user->username;

    sendLine(fd, "OK Logged in as " + username);
}

static bool isValidUsername(const std::string &username)
{
    if (username.empty()) return false;
    for (char c : username)
        if (!isalnum(static_cast<unsigned char>(c)))
            return false;
    return true;
}

void cmdCheckUser(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2) { sendLine(fd, "ERR Usage: CHECK_USER <username>"); return; }
    if (!isValidUsername(args[1])) { sendLine(fd, "ERR Username may only contain letters and numbers"); return; }
    std::lock_guard<std::mutex> lock(state.mtx);
    auto user = state.db.query<User>("username", args[1]);
    sendLine(fd, user ? "FOUND" : "NOT_FOUND");
}

// LOGOUT
void cmdLogout(int fd, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);

    for (auto &[roomName, fds] : state.rooms)
        fds.erase(fd);

    state.sessions[fd].loggedIn = false;
    state.sessions[fd].userId.clear();
    state.sessions[fd].username.clear();

    sendLine(fd, "OK Logged out");
}