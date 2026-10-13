// This fixture deliberately changes behavior; it is not a real compiler defect.
#include "sentinel/io.hpp"
#include "sentinel/process.hpp"
#include <iostream>
int main(int argc,char** argv) {
    try {
        std::vector<std::string> args{REAL_CLANG};
        std::filesystem::path source; bool optimized=false;
        for(int i=1;i<argc;++i) {
            std::string a=argv[i];
            if(a=="--version") { std::cout<<"A64Sentinel synthetic clang fixture 1\n"; return 0; }
            if(a=="-O2" || a=="-O3") optimized=true;
            if(a.size()>2 && (a.substr(a.size()-2)==".c" || (a.size()>4 && a.substr(a.size()-4)==".cpp"))) source=a;
            args.push_back(a);
        }
        if(source.empty()) return 2;
        auto text=sentinel::read_file(source);
        bool bug=optimized && text.find("/* BEGIN block 5 */")!=std::string::npos;
        auto injected=std::filesystem::current_path()/"injected.c";
        sentinel::write_file(injected,"#include <stdio.h>\nint main(void) { puts(\""+std::string(bug?"1":"0")+"\"); return 0; }\n");
        for(auto& a:args) if(a==source.string()) a=injected.string();
        auto r=sentinel::execute(args,std::filesystem::current_path(),std::chrono::seconds(10));
        std::cout<<r.out; std::cerr<<r.err; return r.started && !r.timed_out ? r.exit_code : 2;
    } catch(const std::exception& e) { std::cerr<<e.what(); return 2; }
}
