#include "server.h"
#include <unistd.h>

void handleClient(int fd, ServerState &state)
{
    sendLine(fd, "INFO Welcome to ChatApp. Commands: SIGNUP LOGIN LOGOUT DM JOIN LEAVE MSG CREATE_ROOM LIST_ROOMS LIST_USERS LIST_MEMBERS HISTORY_DM HISTORY_ROOM");

    while (true)
    {
        std::string line = recvLine(fd);
        if (line.empty())
        {
            std::cout << "[server] Client fd=" << fd << " disconnected\n";
            break;
        }

        auto parts = splitMessage(line, ' ', 3); // cmd arg1 rest...

        if (parts.empty())
            continue;

        std::string cmd = parts[0];

        // Commands that don't require login
        if (cmd == "SIGNUP")
        {
            cmdSignup(fd, parts, state);
            continue;
        }
        if (cmd == "LOGIN")
        {
            cmdLogin(fd, parts, state);
            continue;
        }

        // All other commands require login
        {
            std::lock_guard<std::mutex> lock(state.mtx);
            if (!state.sessions[fd].loggedIn)
            {
                sendLine(fd, "ERR Not logged in");
                continue;
            }
        }

        if (cmd == "LOGOUT")
            cmdLogout(fd, state);
        else if (cmd == "DM")
            cmdDm(fd, parts, state);
        else if (cmd == "JOIN")
            cmdJoin(fd, parts, state);
        // In handleClient, replace the DH lines with:
        else if (cmd == "DH_INIT")
            cmdInitDH(fd, splitMessage(line, ' ', 4), state);
        else if (cmd == "DH_REPLY")
            cmdReplyDH(fd, splitMessage(line, ' ', 4), state);
        else if (cmd == "LEAVE")
            cmdLeave(fd, parts, state);
        else if (cmd == "MSG")
            cmdMsg(fd, parts, state);
        else if (cmd == "CREATE_ROOM")
            cmdCreateRoom(fd, parts, state);
        else if (cmd == "LIST_ROOMS")
            cmdListRooms(fd, state);
        else if (cmd == "LIST_MEMBERS")
            cmdListMembers(fd, parts, state);
        else if (cmd == "LIST_USERS")
            cmdListUsers(fd, state);
        else if (cmd == "HISTORY_DM")
            cmdHistoryDm(fd, parts, state);
        else if (cmd == "HISTORY_ROOM")
            cmdHistoryRoom(fd, parts, state);
        else if (cmd == "FILE_SEND")
            cmdFileSend(fd, parts, state);
        else if (cmd == "FILE_ACCEPT")
            cmdFileAccept(fd, parts, state);
        else if (cmd == "FILE_REJECT")
            cmdFileReject(fd, parts, state);
        else if (cmd == "FILE_DATA")
            cmdFileData(fd, parts, state);
        else if (cmd == "FILE_END")
            cmdFileEnd(fd, state);
        else
            sendLine(fd, "ERR Unknown command");
    }

    // Cleanup on disconnect
    std::lock_guard<std::mutex> lock(state.mtx);
    for (auto &[roomName, fds] : state.rooms)
        fds.erase(fd);
    state.sessions.erase(fd);
    close(fd);
}
