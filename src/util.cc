#include <chrono>
#include <unistd.h>
#include "util.hpp"

namespace util {
double getTime() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double t = duration.count() / 1000000.0f;

    return t;
}
}
