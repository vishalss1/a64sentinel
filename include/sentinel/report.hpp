#pragma once
#include "sentinel/testing.hpp"
#include <cstdint>
namespace sentinel {
void report(const std::filesystem::path&, std::uint64_t seed, const std::string& source,
            const std::vector<Observation>&, std::chrono::milliseconds timeout);
void diagnostics(const Configuration&, const std::filesystem::path& source,
                 const std::filesystem::path& work, std::chrono::milliseconds timeout);
}
