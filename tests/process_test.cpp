#include <gtest/gtest.h>
#include "sentinel/process.hpp"
#include <atomic>
namespace {
std::filesystem::path scratch() {
    static std::atomic<unsigned> n{0};
    auto p=std::filesystem::temp_directory_path()/("sentinel test spaces "+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(++n));
    std::filesystem::create_directories(p); return p;
}
}
TEST(Process, PreservesArgumentBoundariesAndCapturesBothStreams) {
    auto dir=scratch();
    auto r=sentinel::execute({PROCESS_FIXTURE,"hello world","quote\"value","trailing\\",""},dir,std::chrono::seconds(2));
    EXPECT_TRUE(r.started); EXPECT_EQ(r.exit_code,7); EXPECT_FALSE(r.timed_out);
    EXPECT_EQ(r.out,"hello world\nquote\"value\ntrailing\\\n\n"); EXPECT_EQ(r.err,"stderr\n");
    std::filesystem::remove_all(dir);
}
TEST(Process, TerminatesTimeoutsAndDetectsTruncation) {
    auto dir=scratch();
    auto r=sentinel::execute({PROCESS_FIXTURE,"sleep"},dir,std::chrono::milliseconds(100));
    EXPECT_TRUE(r.timed_out); EXPECT_LT(r.elapsed_ms,2000);
    r=sentinel::execute({PROCESS_FIXTURE,"large"},dir,std::chrono::seconds(2));
    EXPECT_TRUE(r.truncated); EXPECT_EQ(r.out.size(),1024u*1024u);
    std::filesystem::remove_all(dir);
}
TEST(Process, MissingExecutablesAreUnsuccessful) {
    auto dir=scratch(); auto r=sentinel::execute({"missing-sentinel-executable-abc"},dir,std::chrono::seconds(1));
    EXPECT_TRUE(!r.started || r.exit_code!=0); std::filesystem::remove_all(dir);
}
