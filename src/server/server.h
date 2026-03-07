#pragma once
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../db/Database.hpp"
#include "../db/models/User.hpp"
#include "../db/models/Message.hpp"
#include "../db/models/Chatroom.hpp"
#include "../db/models/Chatroommember.hpp"
#include "../utils/utils.h"

// ---- Protocol ----
// All messages are newline-terminated strings.
// Client → Server commands:
//   SIGNUP <username> <password>
//   LOGIN <username> <password>
//   LOGOUT
//   DM <username> <message...>
//   JOIN <roomName>
//   LEAVE <roomName>
//   MSG <roomName> <message...>
//   CREATE_ROOM <roomName>
//   LIST_ROOMS
//   LIST_MEMBERS <roomName>
//   LIST_USERS
//   HISTORY_DM <username>
//   HISTORY_ROOM <roomName>
//
// Server → Client responses:
//   OK [optional info]
//   ERR <reason>
//   MSG_FROM <from> <message...>           (incoming DM)
//   ROOM_MSG <room> <from> <message...>    (incoming group message)
//   INFO <text>                            (informational notices)

// Session state per connected client
struct ClientSession
{
    int fd;
    std::string userId;   // empty if not logged in
    std::string username; // empty if not logged in
    bool loggedIn = false;
};

struct PendingFileOffer
{
    int      senderFd;
    std::string filename;
    size_t   filesize;
};

// Global server state (shared across threads)
struct ServerState
{
    std::mutex mtx;
    std::map<int, ClientSession> sessions;       // fd → session
    std::map<std::string, std::set<int>> rooms;  // roomName → set of fds currently in room
    Database db;
    std::map<int, PendingFileOffer> pendingFiles;   // receiverFd → offer
    std::map<int, int> pendingCalls;  // receiverFd → callerFd
    std::map<int, int> activeCalls;   // fd → peerFd (both directions)
};

// Socket setup
int  serverSocket();
void serverBind(int sockfd, int port);
void serverListen(int sockfd, int backlog);
int  serverAccept(int sockfd);

// Client handler (runs in its own thread)
void handleClient(int clientFd, ServerState &state);

// Command handlers (called from handleClient)
void cmdSignup(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdLogin(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdLogout(int fd, ServerState &state);
void cmdDm(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdJoin(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdLeave(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdMsg(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdCreateRoom(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdListRooms(int fd, ServerState &state);
void cmdListMembers(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdListUsers(int fd, ServerState &state);
void cmdHistoryDm(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdHistoryRoom(int fd, const std::vector<std::string> &args, ServerState &state);

void cmdFileSend(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdFileAccept(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdFileReject(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdFileData(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdFileEnd(int fd, ServerState &state);

void cmdCall(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdCallAccept(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdCallPort(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdCallReject(int fd, const std::vector<std::string> &args, ServerState &state);
void cmdCallEnd(int fd, const std::vector<std::string> &args, ServerState &state);
