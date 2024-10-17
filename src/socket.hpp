#pragma once

#include <map>
#include <string_view>
#include <netinet/in.h>
#include <sys/socket.h>
#include <tuple>

#include "io.hpp"

namespace io {
class Socket: public IoKey<std::tuple<std::string_view, int>> {
public:
    explicit Socket(const std::string &name, const int port, const int buffer_size);
    ~Socket() override;

    int read(std::tuple<std::string_view, int> &key, char *data) override;
    bool send(const std::tuple<std::string_view, int> &key, const std::string& message);

private:
    int sockfd;
    sockaddr_in my_addr;
    std::map<int, sockaddr_in> clients;
};
}

