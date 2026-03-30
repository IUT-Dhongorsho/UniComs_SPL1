#pragma once
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utils/Utils.h"

int clientConnect(const std::string &host, int port);
void runClient(int fd);