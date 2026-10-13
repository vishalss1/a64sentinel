#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
int main(int argc,char** argv) {
#ifdef _WIN32
    _setmode(_fileno(stdout),_O_BINARY);
    _setmode(_fileno(stderr),_O_BINARY);
#endif
    if(argc>1 && std::string(argv[1])=="sleep") { std::this_thread::sleep_for(std::chrono::seconds(5)); return 0; }
    if(argc>1 && std::string(argv[1])=="large") { std::cout<<std::string(1100000,'x'); return 0; }
    for(int i=1;i<argc;++i) std::cout<<argv[i]<<"\n";
    std::cerr<<"stderr\n"; return 7;
}
