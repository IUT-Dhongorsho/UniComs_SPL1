#include "Server.h"

// JOIN <roomName>
void cmdJoin(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2)
    {
        sendLine(fd, "ERR Usage: JOIN <roomName> [password]");
        return;
    }

    const std::string &roomName = args[1];
    // Extract password if provided, otherwise default to empty string
    const std::string providedPassword = (args.size() >= 3) ? args[2] : "";

    std::lock_guard<std::mutex> lock(state.mtx);

    try {
        auto room = state.db.query<ChatRoom>("name", roomName);
        if (!room)
        {
            sendLine(fd, "ERR Room not found: " + roomName);
            return;
        }

        // If the room has a password stored, enforce it
        if (!room->password.empty()) 
        {
            if (providedPassword.empty()) 
            {
                sendLine(fd, "ERR Room is password protected. Usage: join <roomName> <password>");
                return;
            }
            if (!verifyPassword(providedPassword, room->password)) 
            {
                sendLine(fd, "ERR Incorrect password for room: " + roomName);
                return;
            }
        }
        const auto &sess = state.sessions[fd];
        std::string memberId = room->id + ":" + sess.userId;

        // Add to DB membership if not already a member
        if (!state.db.query<ChatRoomMember>("id", memberId))
            state.db.insert(ChatRoomMember{memberId, room->id, sess.userId});

        // Add to in-memory room (for live delivery)
        state.rooms[roomName].insert(fd);

        // Notify others in room
        for (int ofd : state.rooms[roomName])
            if (ofd != fd)
                sendLine(ofd, "INFO " + sess.username + " joined " + roomName);

        sendLine(fd, "OK Joined " + roomName);
    } catch (const std::exception &e) {
        sendLine(fd, std::string("ERR Database error: ") + e.what());
    }
}

// LEAVE <roomName>
void cmdLeave(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2)
    {
        sendLine(fd, "ERR Usage: LEAVE <roomName>");
        return;
    }

    const std::string &roomName = args[1];
    std::lock_guard<std::mutex> lock(state.mtx);

    state.rooms[roomName].erase(fd);

    const auto &sess = state.sessions[fd];
    for (int ofd : state.rooms[roomName])
        sendLine(ofd, "INFO " + sess.username + " left " + roomName);

    sendLine(fd, "OK Left " + roomName);
}

// MSG <roomName> <message...>
void cmdMsg(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 3)
    {
        sendLine(fd, "ERR Usage: MSG <roomName> <message>");
        return;
    }

    const std::string &roomName = args[1];
    const std::string &content  = args[2];

    std::lock_guard<std::mutex> lock(state.mtx);

    try {
        auto room = state.db.query<ChatRoom>("name", roomName);
        if (!room)
        {
            sendLine(fd, "ERR Room not found: " + roomName);
            return;
        }

        const auto &sess = state.sessions[fd];

        // Must be in room to send
        if (!state.rooms[roomName].count(fd))
        {
            sendLine(fd, "ERR You are not in room: " + roomName);
            return;
        }

        // Persist
        Message msg{generateId(), "group", sess.userId, "", room->id, content, currentTimestamp()};
        state.db.insert(msg);

        // Broadcast to everyone in room except sender
        std::string broadcast = "ROOM_MSG " + roomName + " " + sess.username + " " + content;
        for (int ofd : state.rooms[roomName])
            if (ofd != fd)
                sendLine(ofd, broadcast);

        sendLine(fd, "OK");
    } catch (const std::exception &e) {
        sendLine(fd, std::string("ERR Database error: ") + e.what());
    }
}

// CREATE_ROOM <roomName> <password>
void cmdCreateRoom(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 3)
    {
        sendLine(fd, "ERR Usage: CREATE_ROOM <roomName> <password>");
        return;
    }

    const std::string &roomName = args[1];
    const std::string &password = args[2];
    std::lock_guard<std::mutex> lock(state.mtx);

    try {
        if (state.db.query<ChatRoom>("name", roomName))
        {
            sendLine(fd, "ERR Room already exists: " + roomName);
            return;
        }

        const auto &sess = state.sessions[fd];
        std::string hashedPassword = hashPassword(password);
        ChatRoom room{generateId(), roomName, sess.userId, hashedPassword};
        state.db.insert(room);

        // Auto-join creator
        std::string memberId = room.id + ":" + sess.userId;
        state.db.insert(ChatRoomMember{memberId, room.id, sess.userId});
        state.rooms[roomName].insert(fd);

        sendLine(fd, "OK Room created: " + roomName);
    } catch (const std::exception &e) {
        std::cerr << "[server] Failed to create room: " << e.what() << "\n";
        sendLine(fd, std::string("ERR Internal database error: ") + e.what());
    }
}