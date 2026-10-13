#include "sentinel/io.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace sentinel {
std::string json(const std::string& s) {
    const char* hex="0123456789abcdef"; std::string out="\"";
    for(unsigned char c:s) {
        if(c=='"' || c=='\\') { out+='\\'; out+=static_cast<char>(c); }
        else if(c<32 || c>=127) { out+="\\u00"; out+=hex[c>>4]; out+=hex[c&15]; }
        else out+=static_cast<char>(c);
    }
    return out+'"';
}
std::string read_file(const std::filesystem::path& p) {
    std::ifstream f(p,std::ios::binary); if(!f) throw std::runtime_error("cannot read "+p.string());
    std::ostringstream s; s<<f.rdbuf(); return s.str();
}
void write_file(const std::filesystem::path& p,const std::string& s) {
    std::filesystem::create_directories(p.parent_path());
    std::ofstream f(p,std::ios::binary); f<<s;
    if(!f) throw std::runtime_error("cannot write "+p.string());
}
}
