#pragma once

#include <type_traits>

#include "io.hpp"
#include "util.hpp"

namespace dev {
template <typename IO, typename Enable = void>
class Dev;

template <typename IO>
class Dev<IO, typename std::enable_if<std::is_base_of<io::IoKey<typename IO::io_key>, IO>::value>::type> {
public:
    explicit Dev(const std::string &name, IO &io, const typename IO::io_key io_key):
        io(io),
        io_key(io_key) {
        auto it = io.unpackers.find(io_key);
        if (it != io.unpackers.end()) {
            if constexpr (util::is_streamable<typename IO::io_key>::value) {
                std::cerr << "[Dev<" + name + R"(>] You used duplicate key ")" << io_key << R"(" when binging to IO ")" << io.name << R"("!)" << std::endl;
            } else {
                std::cerr << "[Dev<" + name + R"(>] You used duplicate key when binging to IO ")" << io.name << R"("!)" << std::endl;
            }
        } else {
            io.unpackers.emplace(io_key, [this](const char *data, const int len) -> bool { return unpack(data, len); });
        }
    }
    virtual ~Dev() = default;

    const std::string name;

protected:
    IO &io;
    const typename IO::io_key io_key;

private:
    virtual bool unpack(const char *data, const int len) = 0;
};
}

