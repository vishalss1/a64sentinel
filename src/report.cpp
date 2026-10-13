#include "sentinel/report.hpp"
#include "sentinel/io.hpp"
#include <sstream>
namespace sentinel {
namespace {
std::string array(const std::vector<std::string>& v) {
    std::string s="["; for(auto& x:v) { if(s.size()>1) s+=','; s+=json(x); } return s+"]";
}
std::string process(const ProcessResult& p) {
    std::ostringstream s;
    s<<"{\"started\":"<<(p.started?"true":"false")<<",\"exit_code\":"<<p.exit_code
     <<",\"timed_out\":"<<(p.timed_out?"true":"false")<<",\"truncated\":"<<(p.truncated?"true":"false")
     <<",\"elapsed_ms\":"<<p.elapsed_ms<<",\"stdout\":"<<json(p.out)<<",\"stderr\":"<<json(p.err)<<"}";
    return s.str();
}
}
void report(const std::filesystem::path& path,std::uint64_t seed,const std::string& source,
            const std::vector<Observation>& obs,std::chrono::milliseconds timeout) {
    std::ostringstream s;
    s<<"{\"schema_version\":1,\"seed\":"<<seed<<",\"source\":"<<json(source)
     <<",\"status\":"<<json(classify(obs))<<",\"timeout_ms\":"<<timeout.count()<<",\"observations\":[";
    for(std::size_t i=0;i<obs.size();++i) {
        auto& o=obs[i]; auto& c=o.config; if(i) s<<',';
        s<<"{\"compiler\":"<<json(c.compiler)<<",\"version\":"<<json(o.version)
         <<",\"optimization\":"<<json(c.optimization)<<",\"target\":"<<json(c.target)
         <<",\"cpu\":"<<json(c.cpu)<<",\"cxx\":"<<(c.cxx?"true":"false")
         <<",\"flags\":"<<array(c.flags)<<",\"runner\":"<<array(c.runner)
         <<",\"command\":"<<array(o.command)<<",\"compilation\":"<<process(o.compilation)
         <<",\"execution\":"<<process(o.execution)<<"}";
    }
    s<<"]}\n"; write_file(path,s.str());
}
void diagnostics(const Configuration& c,const std::filesystem::path& source,const std::filesystem::path& work,
                 std::chrono::milliseconds timeout) {
    auto args=compiler_args(c,std::filesystem::absolute(source));
    args.insert(args.end(),{"-S","-o",std::filesystem::absolute(work/"program.s").string()});
    auto result=execute(args,work,timeout);
    write_file(work/"assembly.log",process(result));
    if(c.compiler.find("clang")!=std::string::npos) {
        args=compiler_args(c,std::filesystem::absolute(source));
        args.insert(args.end(),{"-S","-emit-llvm","-o",std::filesystem::absolute(work/"program.ll").string()});
        result=execute(args,work,timeout); write_file(work/"llvm-ir.log",process(result));
    }
}
}
