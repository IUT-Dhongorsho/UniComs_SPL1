#ifndef CLIENT_HEADER_H
#define CLIENT_HEADER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

int Socket();
int Connect(int sockfd, const std::string &host, int port);

#endif