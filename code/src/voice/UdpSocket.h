#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

class UDPSocket {
public:
    int fd = -1;
    int localPort = 0;

    UDPSocket() {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) throw std::runtime_error("UDP socket failed");

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = 0;
        if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
            throw std::runtime_error("UDP bind failed");

        socklen_t len = sizeof(addr);
        getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &len);
        localPort = ntohs(addr.sin_port);

        fcntl(fd, F_SETFL, O_NONBLOCK);
    }

    ~UDPSocket() { if (fd >= 0) close(fd); }

    void send(const std::string &ip, int port, const std::vector<uint8_t> &data) {
        sockaddr_in dest{};
        dest.sin_family = AF_INET;
        dest.sin_port   = htons(static_cast<uint16_t>(port));
        inet_pton(AF_INET, ip.c_str(), &dest.sin_addr);
        sendto(fd, data.data(), data.size(), 0,
               reinterpret_cast<sockaddr *>(&dest), sizeof(dest));
    }

    std::vector<uint8_t> recv() {
        std::vector<uint8_t> buf(1024);
        ssize_t n = ::recv(fd, buf.data(), buf.size(), 0);
        if (n <= 0) return {};
        buf.resize(n);
        return buf;
    }
};