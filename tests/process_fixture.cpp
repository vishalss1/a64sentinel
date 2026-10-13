#include <chrono>
#include <iostream>
#include <thread>
#include <string>
int main(int argc,char** argv) {
    if(argc>1 && std::string(argv[1])=="sleep") { std::this_thread::sleep_for(std::chrono::seconds(5)); return 0; }
    if(argc>1 && std::string(argv[1])=="large") { std::cout<<std::string(1100000,'x'); return 0; }
    for(int i=1;i<argc;++i) std::cout<<argv[i]<<"\n";
    std::cerr<<"stderr\n"; return 7;
}
