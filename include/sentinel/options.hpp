#pragma once
#include "sentinel/testing.hpp"
#include <cstdint>
namespace sentinel {
struct Options {
    std::uint64_t seed=1;
    unsigned count=1, budget=48;
    std::chrono::milliseconds timeout{10000};
    std::filesystem::path out, source;
    std::string mode="run";
    bool minimize=true, cxx=false, help=false;
    std::vector<Configuration> matrix;
};
Options parse(int argc,char** argv);
std::string usage();
}
