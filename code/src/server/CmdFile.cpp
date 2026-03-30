#include "Server.h"

// FILE_SEND <username> <filename> <filesize>
void cmdFileSend(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 4) { sendLine(fd, "ERR Usage: FILE_SEND <username> <filename> <filesize>"); return; }

    const std::string &toUsername = args[1];
    const std::string &filename   = args[2];
    size_t filesize = std::stoul(args[3]);

    std::lock_guard<std::mutex> lock(state.mtx);

    auto toUser = state.db.query<User>("username", toUsername);
    if (!toUser) { sendLine(fd, "ERR User not found"); return; }

    // Find receiver fd
    int receiverFd = -1;
    for (const auto &[ofd, sess] : state.sessions)
        if (sess.loggedIn && sess.userId == toUser->id)
            { receiverFd = ofd; break; }

    if (receiverFd < 0) { sendLine(fd, "ERR User is not online"); return; }

    state.pendingFiles[receiverFd] = {fd, filename, filesize};

    const std::string &fromUsername = state.sessions[fd].username;
    sendLine(receiverFd, "FILE_OFFER " + fromUsername + " " + filename + " " + std::to_string(filesize));
    sendLine(fd, "OK Waiting for " + toUsername + " to accept");
}

// FILE_ACCEPT <username>
void cmdFileAccept(int fd, const std::vector<std::string> &args, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);

    if (state.pendingFiles.find(fd) == state.pendingFiles.end())
        { sendLine(fd, "ERR No pending file offer"); return; }

    auto &offer = state.pendingFiles[fd];
    sendLine(offer.senderFd, "FILE_ACCEPTED");  // tell sender to start
    sendLine(fd, "FILE_INCOMING " + offer.filename + " " + std::to_string(offer.filesize));
}

// FILE_REJECT <username>
void cmdFileReject(int fd, const std::vector<std::string> &args, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);

    if (state.pendingFiles.find(fd) == state.pendingFiles.end())
        { sendLine(fd, "ERR No pending file offer"); return; }

    sendLine(state.pendingFiles[fd].senderFd, "FILE_REJECTED");
    state.pendingFiles.erase(fd);
}

// FILE_DATA <base64_chunk> — sender relays a chunk, server forwards to receiver
void cmdFileData(int fd, const std::vector<std::string> &args, ServerState &state)
{
    if (args.size() < 2) return;

    std::lock_guard<std::mutex> lock(state.mtx);

    // Find the receiver who has a pending offer from this sender
    for (const auto &[receiverFd, offer] : state.pendingFiles)
        if (offer.senderFd == fd)
            { sendLine(receiverFd, "FILE_DATA " + args[1]); return; }
}

// FILE_END — sender signals transfer complete
void cmdFileEnd(int fd, ServerState &state)
{
    std::lock_guard<std::mutex> lock(state.mtx);

    for (auto it = state.pendingFiles.begin(); it != state.pendingFiles.end(); ++it)
        if (it->second.senderFd == fd)
        {
            sendLine(it->first, "FILE_END");
            state.pendingFiles.erase(it);
            break;
        }

    sendLine(fd, "OK File sent");
}