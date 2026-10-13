#pragma once
#include <functional>
#include <string>
namespace sentinel {
struct Reduction { std::string source; unsigned attempts=0, removed=0; bool confirmed=false; };
Reduction minimize(std::string source,const std::function<bool(const std::string&)>& interesting,unsigned budget=48);
}
