#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <tuple>

#include "io.hpp"

namespace io {
class Socket: public IoKey<std::tuple<in_addr_t, int>> {
public:
    explicit Socket(const std::string &name, const int port, const int buffer_size);
    ~Socket() override;

    int read(std::tuple<in_addr_t, int> &key, char *data) override;
    bool send(const std::tuple<in_addr_t, int> &key, const char *message, const int len);

private:
    int sockfd;
    sockaddr_in my_addr;
};
}

