#include "sentinel/minimize.hpp"
#include <vector>
#include <algorithm>
namespace sentinel {
Reduction minimize(std::string source,const std::function<bool(const std::string&)>& interesting,unsigned budget) {
    Reduction r{source,0,0};
    if(!budget || !interesting(source)) return r;
    ++r.attempts;
    std::size_t width=16;
    while(width) {
        bool progress=true;
        while(progress && r.attempts<budget) {
            progress=false;
            std::vector<std::pair<std::size_t,std::size_t>> spans;
            std::size_t pos=0;
            while((pos=r.source.find("/* BEGIN block ",pos))!=std::string::npos) {
                auto end=r.source.find("/* END block */",pos);
                if(end==std::string::npos) break;
                end+=15; if(end<r.source.size() && r.source[end]=='\n') ++end;
                spans.emplace_back(pos,end); pos=end;
            }
            for(std::size_t i=0;i<spans.size() && r.attempts<budget;i+=width) {
                auto last=std::min(i+width,spans.size())-1;
                auto candidate=r.source; candidate.erase(spans[i].first,spans[last].second-spans[i].first);
                ++r.attempts;
                if(interesting(candidate)) {
                    r.source=std::move(candidate); r.removed+=static_cast<unsigned>(last-i+1); progress=true; break;
                }
            }
        }
        width/=2;
    }
    return r;
}
}
