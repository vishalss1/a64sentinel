#include "sentinel/process.hpp"
#include <fstream>
#include <atomic>
#include <stdexcept>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <thread>
#endif
namespace sentinel {
namespace {
std::string read(const std::filesystem::path& p, bool& truncated) {
    std::ifstream f(p, std::ios::binary);
    std::string s(1024*1024, '\0'); f.read(s.data(), static_cast<std::streamsize>(s.size()));
    s.resize(static_cast<std::size_t>(f.gcount()));
    if(f.peek()!=std::char_traits<char>::eof()) truncated=true;
    return s;
}
#ifdef _WIN32
std::string quote(const std::string& s) {
    std::string q="\""; unsigned slashes=0;
    for(char c:s) {
        if(c=='\\') { ++slashes; continue; }
        q.append(c=='"' ? slashes*2+1 : slashes, '\\'); slashes=0; q+=c;
    }
    q.append(slashes*2, '\\'); return q+'"';
}
#endif
}
ProcessResult execute(const std::vector<std::string>& args, const std::filesystem::path& directory,
                      std::chrono::milliseconds timeout) {
    if(args.empty() || timeout.count()<=0) throw std::invalid_argument("invalid process request");
    std::filesystem::create_directories(directory);
    static std::atomic<unsigned> serial{0};
    auto base=directory/("process-"+std::to_string(++serial));
    auto stdout_path=base.string()+".stdout", stderr_path=base.string()+".stderr";
    auto begin=std::chrono::steady_clock::now(); ProcessResult result;
#ifdef _WIN32
    SECURITY_ATTRIBUTES sa{sizeof(sa),nullptr,TRUE};
    HANDLE out=CreateFileA(stdout_path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    HANDLE err=CreateFileA(stderr_path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    HANDLE input=CreateFileA("NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,OPEN_EXISTING,0,nullptr);
    if(out==INVALID_HANDLE_VALUE || err==INVALID_HANDLE_VALUE || input==INVALID_HANDLE_VALUE) {
        if(out!=INVALID_HANDLE_VALUE) CloseHandle(out);
        if(err!=INVALID_HANDLE_VALUE) CloseHandle(err);
        if(input!=INVALID_HANDLE_VALUE) CloseHandle(input);
        throw std::runtime_error("cannot create process streams");
    }
    STARTUPINFOA si{}; si.cb=sizeof(si); si.dwFlags=STARTF_USESTDHANDLES;
    si.hStdOutput=out; si.hStdError=err; si.hStdInput=input;
    std::string command; for(auto& a:args) { if(!command.empty()) command+=' '; command+=quote(a); }
    PROCESS_INFORMATION pi{};
    HANDLE job=CreateJobObjectA(nullptr,nullptr);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    bool job_ready=job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits));
    if(job_ready && CreateProcessA(nullptr,command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,nullptr,
                      directory.string().c_str(),&si,&pi)) {
        if(AssignProcessToJobObject(job,pi.hProcess)) {
            result.started=true; ResumeThread(pi.hThread);
            if(WaitForSingleObject(pi.hProcess,static_cast<DWORD>(timeout.count()))==WAIT_TIMEOUT) {
                result.timed_out=true; TerminateJobObject(job,124); WaitForSingleObject(pi.hProcess,INFINITE);
            }
            DWORD code; GetExitCodeProcess(pi.hProcess,&code); result.exit_code=static_cast<int>(code);
        } else { TerminateProcess(pi.hProcess,125); result.err="cannot assign process to job"; }
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    } else result.err="cannot launch process: "+std::to_string(GetLastError());
    if(job) CloseHandle(job);
    CloseHandle(out); CloseHandle(err); CloseHandle(input);
#else
    result.err="POSIX backend unavailable";
#endif
    result.out=read(stdout_path,result.truncated); result.err+=read(stderr_path,result.truncated);
    result.elapsed_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-begin).count();
    std::filesystem::remove(stdout_path); std::filesystem::remove(stderr_path);
    return result;
}
}
