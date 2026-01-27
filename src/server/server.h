#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../db/DB.h"
#include <string>
#include <set>

using sockList = std::set<int>;
using sockToName = std::map<int, std::string>;
using chatRoomToSockList = std::map<std::string, sockList>;

int Socket();
void Bind(int sockfd, int port);
void Listen(int sockfd, int backlog);
int Accept(int sockfd);

std::string login(const Database<User> &db, const std::string &uname, const std::string &pass);
bool logout(const Database<User> &db, const std::string &uname);
void joinChatRoom(const std::string roomName, const int clientSock, sockToName &name, chatRoomToSockList &chatRooms);
void leaveChatRoom(const std::string roomName, const int clientSock, const sockToName &names, chatRoomToSockList &chatRooms);
std::string getPeopleList(const std::string &roomName, const sockToName &names, const chatRoomToSockList &chatRooms);
std::string getClientInfo(const int clientSock);
void handleMsg(const int currentClientSock, const chatRoomToSockList &chatRooms, const sockToName &clients, std::string msg);
std::string getChatroomList(const chatRoomToSockList &chatRooms);

#endif
