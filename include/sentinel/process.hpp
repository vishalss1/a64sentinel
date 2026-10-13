#pragma once
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>
namespace sentinel {
struct ProcessResult {
    int exit_code = -1;
    bool timed_out = false;
    bool started = false;
    bool truncated = false;
    std::string out, err;
    long long elapsed_ms = 0;
};
ProcessResult execute(const std::vector<std::string>& args, const std::filesystem::path& directory,
                      std::chrono::milliseconds timeout);
}
