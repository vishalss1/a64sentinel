#include <algorithm>
#include <gtest/gtest.h>
#include "sentinel/generator.hpp"
#include "sentinel/random.hpp"
#include "sentinel/minimize.hpp"
#include "sentinel/testing.hpp"
#include "sentinel/io.hpp"
TEST(Random, StableAndBounded) {
    sentinel::Random r(0); EXPECT_EQ(r.next(),UINT64_C(0xe220a8397b1dcdaf));
    for(int i=0;i<100;++i) EXPECT_LT(r.bounded(7),7u);
    EXPECT_THROW(r.bounded(0),std::invalid_argument);
}
TEST(Generator, RepeatableWithIndependentBlocks) {
    EXPECT_EQ(sentinel::generate(42),sentinel::generate(42));
    EXPECT_NE(sentinel::generate(42),sentinel::generate(43));
    EXPECT_NE(sentinel::generate(42).find("/* BEGIN block 5 */"),std::string::npos);
    EXPECT_NE(sentinel::generate(42,true).find("alignas(64)"),std::string::npos);
}
TEST(Json, EscapesBinaryAndControlBytes) {
    EXPECT_EQ(sentinel::json(std::string("\0\n\"\\",4)),"\"\\u0000\\u000a\\\"\\\\\"");
}
sentinel::Observation valid(std::string output="ok") {
    sentinel::Observation o; o.compilation.started=o.execution.started=true;
    o.compilation.exit_code=o.execution.exit_code=0; o.execution.out=output; return o;
}
TEST(Classification, OutputsExitsAndStderrMatter) {
    auto a=valid(),b=valid(); EXPECT_EQ(sentinel::classify({a,b}),"pass");
    b.execution.out="other"; EXPECT_EQ(sentinel::classify({a,b}),"mismatch");
    b=a; b.execution.exit_code=1; EXPECT_EQ(sentinel::classify({a,b}),"mismatch");
    b=a; b.execution.err="diagnostic"; EXPECT_EQ(sentinel::classify({a,b}),"mismatch");
}
TEST(Classification, FailuresCannotBecomeMiscompilations) {
    auto a=valid(),b=valid(); b.compilation.exit_code=1;
    EXPECT_EQ(sentinel::classify({a,b}),"compile_error");
    b=a; b.execution.timed_out=true; EXPECT_EQ(sentinel::classify({a,b}),"inconclusive");
    b=a; b.execution.truncated=true; EXPECT_EQ(sentinel::classify({a,b}),"inconclusive");
    a.execution.exit_code=b.execution.exit_code=1; b.execution.truncated=false;
    EXPECT_EQ(sentinel::classify({a,b}),"runtime_error");
}
TEST(Reduction, RetainsRequiredBlockAndHonorsBudget) {
    auto source=sentinel::generate(1);
    auto predicate=[](const std::string& s){return s.find("/* BEGIN block 5 */")!=std::string::npos;};
    auto reduced=sentinel::minimize(source,predicate,48);
    EXPECT_TRUE(reduced.confirmed); EXPECT_EQ(reduced.removed,11u); EXPECT_LE(reduced.attempts,48u); EXPECT_TRUE(predicate(reduced.source));
    EXPECT_EQ(sentinel::minimize(source,predicate,0).source,source);
    EXPECT_LE(sentinel::minimize(source,predicate,1).attempts,1u);
}
TEST(Reduction, RejectsUnconfirmedFailures) {
    auto source=sentinel::generate(1);
    EXPECT_EQ(sentinel::minimize(source,[](const std::string&){return false;}).source,source);
}
TEST(Configuration, AddsAArch64AndCpuFlags) {
    sentinel::Configuration c; c.compiler="clang"; c.target="aarch64"; c.cpu="neoverse-v2";
    auto args=sentinel::compiler_args(c,"source.c");
    EXPECT_NE(std::find(args.begin(),args.end(),"--target=aarch64-linux-gnu"),args.end());
    EXPECT_NE(std::find(args.begin(),args.end(),"-mcpu=neoverse-v2"),args.end());
}
