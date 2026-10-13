#include "sentinel/testing.hpp"
namespace sentinel {
std::vector<std::string> compiler_args(const Configuration& c, const std::filesystem::path& source) {
    std::vector<std::string> a{c.compiler, c.cxx ? "-std=c++17" : "-std=c11", "-x", c.cxx ? "c++" : "c", c.optimization};
    if(c.target=="aarch64" && c.compiler.find("clang")!=std::string::npos) a.push_back("--target=aarch64-linux-gnu");
    if(!c.cpu.empty()) a.push_back("-mcpu="+c.cpu);
    a.insert(a.end(),c.flags.begin(),c.flags.end()); a.push_back(source.string()); return a;
}
Observation test(const Configuration& c, const std::filesystem::path& source,
                 const std::filesystem::path& work, std::chrono::milliseconds timeout) {
    std::filesystem::create_directories(work);
    Observation o; o.config=c;
    o.version=execute({c.compiler,"--version"},work,timeout).out;
    auto binary=std::filesystem::absolute(work/"program.exe");
    o.command=compiler_args(c,std::filesystem::absolute(source));
    o.command.insert(o.command.end(),{"-o",binary.string()});
    o.compilation=execute(o.command,work,timeout);
    if(o.compilation.started && !o.compilation.timed_out && o.compilation.exit_code==0) {
        auto run=c.runner; run.push_back(binary.string()); o.execution=execute(run,work,timeout);
    }
    return o;
}
std::string classify(const std::vector<Observation>& observations) {
    if(observations.size()<2) return "inconclusive";
    for(auto& o:observations) {
        if(!o.compilation.started || o.compilation.timed_out || o.compilation.truncated) return "inconclusive";
        if(o.compilation.exit_code!=0) return "compile_error";
        if(!o.execution.started || o.execution.timed_out || o.execution.truncated) return "inconclusive";
    }
    const auto& first=observations.front().execution;
    for(auto& o:observations)
        if(first.exit_code!=o.execution.exit_code || first.out!=o.execution.out || first.err!=o.execution.err) return "mismatch";
    return first.exit_code==0 ? "pass" : "runtime_error";
}
}
