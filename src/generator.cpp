#include "sentinel/generator.hpp"
#include "sentinel/random.hpp"
#include <sstream>
namespace sentinel {
std::string generate(std::uint64_t seed, bool cxx) {
    Random r(seed);
    std::ostringstream s;
    s << "/* A64Sentinel seed " << seed << " language " << (cxx ? "C++" : "C") << " */\n"
      << "#include <stdint.h>\n#include <stdio.h>\n#include <inttypes.h>\n"
      << (cxx ? "#define ALIGN64 alignas(64)\n" : "#define ALIGN64 _Alignas(64)\n")
      << "int main(void) {\nALIGN64 uint32_t a[128], b[128];\n"
      << "uint32_t x = UINT32_C(" << static_cast<std::uint32_t>(r.next()) << ");\n"
      << "for (unsigned i=0;i<128;++i) { a[i]=x+i; b[i]=x^i; }\n";
    for (unsigned j=0;j<12;++j) {
        s << "/* BEGIN block " << j << " */\n"
          << "x = (x * UINT32_C(" << static_cast<std::uint32_t>(r.next()) << ")) + UINT32_C(" << static_cast<std::uint32_t>(r.next()) << ");\n"
          << "x ^= (x >> " << 1+r.bounded(31) << ");\n"
          << "for (unsigned i=0;i<128;++i) a[i] = a[i] + b[i] * UINT32_C(3) + x;\n"
          << "{ uint32_t *p=a; for(unsigned i=1;i<128;++i) p[i] ^= p[i-1] >> 3; }\n"
          << "for(unsigned i=0;i<127;++i) b[i] = a[i+1] ^ (b[i] / UINT32_C(7));\n"
          << "x = (x << " << 1+r.bounded(15) << ") | (x >> " << 16+r.bounded(15) << ");\n"
          << "for(unsigned i=0;i<128;++i) { if((a[i]&3u)==0) b[i]+=x; else b[i]^=a[i]; }\n"
          << "{ unsigned char *bytes=(unsigned char*)a; x += bytes[" << r.bounded(128) << "]; }\n"
          << "/* END block */\n";
    }
    s << "uint64_t hash=x;\nfor(unsigned i=0;i<128;++i) hash=(hash^a[i]^b[i])*UINT64_C(1099511628211);\n"
      << "printf(\"%\" PRIu64 \"\\n\", hash);\nreturn 0;\n}\n";
    return s.str();
}
}
