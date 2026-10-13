#include "sentinel/generator.hpp"
#include "sentinel/options.hpp"
#include "sentinel/report.hpp"
#include "sentinel/minimize.hpp"
#include "sentinel/io.hpp"
#include <iostream>
namespace {
std::vector<sentinel::Observation> run(const sentinel::Options& o,const std::filesystem::path& source,const std::filesystem::path& work,bool artifacts) {
    std::vector<sentinel::Observation> obs;
    for(std::size_t i=0;i<o.matrix.size();++i) {
        auto dir=work/("config-"+std::to_string(i));
        obs.push_back(sentinel::test(o.matrix[i],source,dir,o.timeout));
        if(artifacts) sentinel::diagnostics(o.matrix[i],source,dir,o.timeout);
    }
    return obs;
}
}
int main(int argc,char** argv) {
    try {
        auto o=sentinel::parse(argc,argv);
        if(o.help) { std::cout<<sentinel::usage(); return 0; }
        if(o.out.empty()) o.out=std::filesystem::path("out")/("run-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
        o.out=std::filesystem::absolute(o.out);
        unsigned mismatches=0, errors=0;
        for(unsigned i=0;i<o.count;++i) {
            auto seed=o.seed+i; auto dir=o.out/("seed-"+std::to_string(seed));
            if(std::filesystem::exists(dir)) throw std::runtime_error("case directory already exists: "+dir.string());
            auto source=dir/(o.cxx ? "source.cpp" : "source.c");
            auto text=o.source.empty() ? sentinel::generate(seed,o.cxx) : sentinel::read_file(o.source);
            sentinel::write_file(source,text);
            if(o.mode=="generate") { std::cout<<source.string()<<"\n"; continue; }
            auto observations=run(o,source,dir/"original",false);
            auto status=sentinel::classify(observations);
            sentinel::report(dir/"report.json",seed,source.filename().string(),observations,o.timeout);
            if(status!="pass") {
                for(std::size_t j=0;j<o.matrix.size();++j)
                    sentinel::diagnostics(o.matrix[j],source,dir/"original"/("config-"+std::to_string(j)),o.timeout);
            }
            if(status=="mismatch" && o.minimize) {
                std::size_t left=0,right=1;
                auto different=[](const sentinel::Observation& a,const sentinel::Observation& b) {
                    return a.execution.exit_code!=b.execution.exit_code || a.execution.out!=b.execution.out || a.execution.err!=b.execution.err;
                };
                for(std::size_t j=1;j<observations.size();++j) if(different(observations[0],observations[j])) { right=j; break; }
                auto candidate=dir/(o.cxx ? "candidate.cpp" : "candidate.c");
                unsigned attempt=0;
                auto predicate=[&](const std::string& trial) {
                    sentinel::write_file(candidate,trial);
                    for(unsigned repeat=0;repeat<2;++repeat) {
                        auto work=dir/"reduction"/("trial-"+std::to_string(attempt++));
                        std::vector<sentinel::Observation> pair{
                            sentinel::test(o.matrix[left],candidate,work/"left",o.timeout),
                            sentinel::test(o.matrix[right],candidate,work/"right",o.timeout)};
                        if(sentinel::classify(pair)!="mismatch") return false;
                    }
                    return true;
                };
                auto reduced=sentinel::minimize(text,predicate,o.budget);
                auto minimized=dir/(o.cxx ? "minimized.cpp" : "minimized.c");
                sentinel::write_file(minimized,reduced.source);
                auto final=run(o,minimized,dir/"minimized",true);
                sentinel::report(dir/"minimized-report.json",seed,minimized.filename().string(),final,o.timeout);
                sentinel::write_file(dir/"reduction.json","{\"attempts\":"+std::to_string(reduced.attempts)+",\"removed_blocks\":"+std::to_string(reduced.removed)+",\"budget\":"+std::to_string(o.budget)+"}\n");
                std::filesystem::remove(candidate);
                std::filesystem::remove_all(dir/"reduction");
            }
            if(status=="mismatch") ++mismatches;
            else if(status!="pass") ++errors;
            std::cout<<"seed "<<seed<<": "<<status<<" ("<<dir.string()<<")\n";
        }
        return mismatches ? 1 : errors ? 2 : 0;
    } catch(const std::exception& e) { std::cerr<<"a64sentinel: "<<e.what()<<"\n"; return 2; }
}
