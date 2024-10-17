#include <arpa/inet.h>
#include <netdb.h>

#include "socket.hpp"

namespace io {
Socket::Socket(const std::string &name, const int port, const int buffer_size):
    IoKey(name, buffer_size),
    sockfd(socket(AF_INET, SOCK_DGRAM, 0)) {
    if (sockfd < 0) {
        std::cerr << "[IO<" + name + ">] Cannot open socket.";
        return;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        std::cerr << "[IO<" + name + ">] Cannot bind socket fd with port number" << port << "." << std::endl;
    }
}

Socket::~Socket() {
    close(sockfd);
}

int Socket::read(std::tuple<std::string_view, int> &key, char *data) {
    sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    auto len = recvfrom(sockfd, data, 256, MSG_WAITALL, (sockaddr *)&cli_addr, &cli_addr_len);
    if (len > 0) {
        char ip_str[16];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
        int port = ntohs(cli_addr.sin_port);
        key = std::make_tuple(ip_str, port);
    }
    return len;
}

bool Socket::send(const std::tuple<std::string_view, int> &key, const std::string& message) {
    std::string_view ip = std::get<0>(key);
    int port = std::get<1>(key);

    sockaddr_in to_addr;
    memset(&to_addr, 0, sizeof(to_addr));
    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.data(), &to_addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address: " << ip << std::endl << std::endl;
        return false;
    }

    auto n = sendto(sockfd, message.c_str(), message.size(), MSG_CONFIRM, reinterpret_cast<const sockaddr*>(&to_addr), sizeof(to_addr));
    if (n < 0) {
        std::clog << "[IO<" + name + ">] Failed to send to client, key: " << ip << ":" << port << "." << std::endl;
        return false;
    }

    return true;
}
}

