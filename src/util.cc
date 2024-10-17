#include <chrono>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "util.hpp"

namespace util {
double getTime() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double t = duration.count() / 1000000.0f;

    return t;
}

in_addr_t to_in_addr(const std::string &host) {
    in_addr_t ip;
    addrinfo hints, *res;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((status = getaddrinfo(host.c_str(), nullptr, &hints, &res)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return INADDR_NONE;
    }

    sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in *>(res->ai_addr);
    ip = ipv4->sin_addr.s_addr;

    if (res->ai_next != nullptr) {
        std::clog << R"(The host ")" + host + R"(" corresponds to multiple IP addresses)";
    }

    freeaddrinfo(res);

    return ip;
}
}
