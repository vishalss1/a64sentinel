#pragma once
#include <filesystem>
#include <string>
namespace sentinel {
std::string json(const std::string&);
std::string read_file(const std::filesystem::path&);
void write_file(const std::filesystem::path&, const std::string&);
}
