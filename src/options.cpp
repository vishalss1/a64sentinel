#include "sentinel/options.hpp"
#include <stdexcept>
#include <limits>
namespace sentinel {
std::string usage() {
    return "A64Sentinel --mode run|generate --seed N --count N --out DIR\n"
           "  --compiler PATH (repeat) --opt -O0|-O2|-O3 (repeat) --cxx\n"
           "  --target native|aarch64 --cpu CPU --flag ARG (repeat)\n"
           "  --runner EXEC --runner-arg ARG (repeat) --source FILE\n"
           "  --timeout-ms N --budget N --no-minimize --help\n";
}
Options parse(int argc,char** argv) {
    Options o; Configuration base; std::vector<std::string> compilers,opts;
    auto number=[](const std::string& s) {
        if(s.empty() || s.find_first_not_of("0123456789")!=std::string::npos) throw std::invalid_argument("expected nonnegative integer");
        std::size_t used; auto n=std::stoull(s,&used); return n;
    };
    for(int i=1;i<argc;++i) {
        std::string key=argv[i];
        if(key=="--help") { o.help=true; continue; }
        if(key=="--cxx") { o.cxx=base.cxx=true; continue; }
        if(key=="--no-minimize") { o.minimize=false; continue; }
        if(i+1>=argc) throw std::invalid_argument("missing value for "+key);
        std::string v=argv[++i];
        if(key=="--compiler") compilers.push_back(v);
        else if(key=="--opt") { if(v!="-O0" && v!="-O2" && v!="-O3") throw std::invalid_argument("unsupported optimization"); opts.push_back(v); }
        else if(key=="--mode") o.mode=v;
        else if(key=="--seed") o.seed=number(v);
        else if(key=="--count" || key=="--budget" || key=="--timeout-ms") {
            auto n=number(v); if(n>1000000 || (key!="--budget" && n==0)) throw std::invalid_argument("numeric option out of range");
            if(key=="--count") o.count=static_cast<unsigned>(n);
            else if(key=="--budget") o.budget=static_cast<unsigned>(n);
            else o.timeout=std::chrono::milliseconds(n);
        }
        else if(key=="--out") o.out=v;
        else if(key=="--source") o.source=v;
        else if(key=="--target") base.target=v;
        else if(key=="--cpu") base.cpu=v;
        else if(key=="--flag") base.flags.push_back(v);
        else if(key=="--runner") { if(!base.runner.empty()) throw std::invalid_argument("runner already set"); base.runner.push_back(v); }
        else if(key=="--runner-arg") { if(base.runner.empty()) throw std::invalid_argument("set runner before runner arguments"); base.runner.push_back(v); }
        else throw std::invalid_argument("unknown option "+key);
    }
    if(o.mode!="run" && o.mode!="generate") throw std::invalid_argument("invalid mode");
    if(base.target!="native" && base.target!="aarch64") throw std::invalid_argument("invalid target");
    if(!o.source.empty() && o.count!=1) throw std::invalid_argument("source replay requires count 1");
    if(o.count-1>std::numeric_limits<std::uint64_t>::max()-o.seed) throw std::invalid_argument("seed range overflow");
    if(compilers.empty()) compilers={"gcc","clang"};
    if(opts.empty()) opts={"-O0","-O2","-O3"};
    auto normalize=[](std::string& path) {
        if(std::filesystem::path(path).has_parent_path()) path=std::filesystem::absolute(path).string();
    };
    for(auto& compiler:compilers) normalize(compiler);
    if(!base.runner.empty()) normalize(base.runner.front());
    for(auto& compiler:compilers) for(auto& opt:opts) { auto c=base; c.compiler=compiler; c.optimization=opt; o.matrix.push_back(c); }
    if(o.mode=="run" && o.matrix.size()<2) throw std::invalid_argument("differential testing requires two configurations");
    return o;
}
}
