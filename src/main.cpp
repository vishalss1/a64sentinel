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
            if(status=="mismatch") ++mismatches;
            else if(status!="pass") ++errors;
            std::cout<<"seed "<<seed<<": "<<status<<" ("<<dir.string()<<")\n";
        }
        return mismatches ? 1 : errors ? 2 : 0;
    } catch(const std::exception& e) { std::cerr<<"a64sentinel: "<<e.what()<<"\n"; return 2; }
}
