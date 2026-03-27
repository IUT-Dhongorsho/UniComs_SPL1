#include "Server.h"

// LIST_ROOMS
void cmdListRooms(int fd, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);
    auto rooms = state.db.getAll<ChatRoom>("ChatRoom");

    if (rooms.empty())
    {
        sendLine(fd, "INFO No rooms available");
        return;
    }

    std::string result = "INFO Rooms:";
    for (const auto &r : rooms)
        result += " [" + r.name + "]";
    sendLine(fd, result);
}

// LIST_MEMBERS <roomName>
void cmdListMembers(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2)
    {
        sendLine(fd, "ERR Usage: LIST_MEMBERS <roomName>");
        return;
    }

    const std::string &roomName = args[1];
    std::lock_guard<std::mutex> lock(state.mtx);

    auto room = state.db.query<ChatRoom>("name", roomName);
    if (!room)
    {
        sendLine(fd, "ERR Room not found: " + roomName);
        return;
    }

    auto members = state.db.queryAll<ChatRoomMember>("ChatRoomMember", "roomId", room->id);

    if (members.empty())
    {
        sendLine(fd, "INFO No members in " + roomName);
        return;
    }

    std::string result = "INFO Members of " + roomName + ":";
    for (const auto &m : members)
    {
        auto user = state.db.query<User>("id", m.userId);
        if (user)
            result += " " + user->username;
    }
    sendLine(fd, result);
}

// LIST_USERS
void cmdListUsers(int fd, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);
    auto users = state.db.getAll<User>("User");

    std::string result = "INFO Users:";
    for (const auto &u : users)
        result += " " + u.username;
    sendLine(fd, result);
}

// HISTORY_DM <username>
void cmdHistoryDm(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2)
    {
        sendLine(fd, "ERR Usage: HISTORY_DM <username>");
        return;
    }

    const std::string &otherUsername = args[1];
    std::lock_guard<std::mutex> lock(state.mtx);

    const auto &sess = state.sessions[fd];
    auto otherUser = state.db.query<User>("username", otherUsername);
    if (!otherUser)
    {
        sendLine(fd, "ERR User not found: " + otherUsername);
        return;
    }

    auto allMessages = state.db.getAll<Message>("Message");
    bool found = false;
    for (const auto &msg : allMessages)
    {
        if (msg.type != "dm")
            continue;
        bool sent     = (msg.fromId == sess.userId   && msg.toId == otherUser->id);
        bool received = (msg.fromId == otherUser->id && msg.toId == sess.userId);
        if (!sent && !received)
            continue;

        std::string from = sent ? sess.username : otherUsername;
        sendLine(fd, "INFO [" + msg.timestamp + "] " + from + ": " + msg.content);
        found = true;
    }

    if (!found)
        sendLine(fd, "INFO No DM history with " + otherUsername);
}

// HISTORY_ROOM <roomName>
void cmdHistoryRoom(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2)
    {
        sendLine(fd, "ERR Usage: HISTORY_ROOM <roomName>");
        return;
    }

    const std::string &roomName = args[1];
    std::lock_guard<std::mutex> lock(state.mtx);

    auto room = state.db.query<ChatRoom>("name", roomName);
    if (!room)
    {
        sendLine(fd, "ERR Room not found: " + roomName);
        return;
    }

    auto messages = state.db.queryAll<Message>("Message", "roomId", room->id);
    if (messages.empty())
    {
        sendLine(fd, "INFO No messages in " + roomName);
        return;
    }

    for (const auto &msg : messages)
    {
        auto user = state.db.query<User>("id", msg.fromId);
        std::string username = user ? user->username : "unknown";
        sendLine(fd, "INFO [" + msg.timestamp + "] " + username + ": " + msg.content);
    }
}