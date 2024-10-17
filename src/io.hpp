#pragma once

#include <cstring>
#include <functional>
#include <string>
#include <thread>
#include <map>

#include "util.hpp"

namespace io {
class Io {
public:
    explicit Io(const std::string &name, const int buffer_size):
        name(name),
        buffer_size(buffer_size),
        buffer(new char[buffer_size]) {
    }
    virtual ~Io() {
        delete[] buffer;
        if (thread && thread->joinable()) {
            thread->join();
        }
    }

    void run() {
        if (thread == nullptr) {
            std::cerr << "[IO<" + name + ">] Repeated run an IO!" << std::endl;
            return;
        }
        running = true;
        thread = new std::thread([this]() { thread_func(); });
    }
    void stop() {
        running = false;
        if (thread != nullptr) {
            thread->join();
            std::cout << "[IO<" + name + ">] Stopped!" << std::endl;
        }
    }

    const std::string name;

protected:
    const int buffer_size;
    char *buffer;
    std::atomic<bool> running {false};

    virtual void thread_func() = 0;

private:
    std::thread *thread {nullptr};

};

template <typename Tkey>
class IoKey: public Io {
public:
    explicit IoKey(const std::string &name, const int buffer_size):
        Io(name, buffer_size) {
    }
    ~IoKey() override = default;

    using key_type = Tkey;

    std::map<Tkey, std::function<void (const char *, const int len)>> unpackers;

    virtual int read(Tkey &key, char *data) = 0;

private:
    std::atomic<bool> running;

    void thread_func() override {
        while (running) {
            int len;
            Tkey key;
            len = read(key, buffer);
            if (len <= 0) {
                std::clog << "[IO<" + name + ">] Read Error!" << std::endl;
            } else {
                auto it = unpackers.find(key);
                if (it == unpackers.end()) {
                    if constexpr (util::is_streamable<Tkey>::value) {
                        std::clog << "[IO<" + name + ">] Unbound key read: " << key << "." << std::endl;
                    } else {
                        std::clog << "[IO<" + name + ">] Unbound key read." << std::endl;
                    }
                } else {
                    it->second(buffer, len);
                }
            }
        }
    }
};
}

