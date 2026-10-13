#pragma once
#include "sentinel/process.hpp"
namespace sentinel {
struct Configuration {
    std::string compiler, optimization="-O2", target="native", cpu;
    std::vector<std::string> flags, runner;
    bool cxx=false;
};
struct Observation {
    Configuration config;
    std::vector<std::string> command, run_command;
    std::string work_directory;
    ProcessResult compilation, execution;
    std::string version, target_triple;
};
Observation test(const Configuration&, const std::filesystem::path& source,
                 const std::filesystem::path& work, std::chrono::milliseconds timeout);
std::string classify(const std::vector<Observation>&);
std::vector<std::string> compiler_args(const Configuration&, const std::filesystem::path&);
}
