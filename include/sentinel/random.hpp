#pragma once
#include <cstdint>
#include <stdexcept>
namespace sentinel {
class Random {
    std::uint64_t state_;
public:
    explicit Random(std::uint64_t seed): state_(seed) {}
    std::uint64_t next() {
        auto z = (state_ += UINT64_C(0x9e3779b97f4a7c15));
        z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
        return z ^ (z >> 31);
    }
    std::uint64_t bounded(std::uint64_t n) {
        if (!n) throw std::invalid_argument("empty random range");
        return next() % n;
    }
};
}
