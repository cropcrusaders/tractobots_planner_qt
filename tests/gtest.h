#ifndef MINI_GTEST_H
#define MINI_GTEST_H
#include <vector>
#include <functional>
#include <cassert>
namespace testing {
using TestFunc = std::function<void()>;
inline std::vector<TestFunc>& registry() { static std::vector<TestFunc> r; return r; }
inline int RunAllTests(){ for(auto&f:registry()) f(); return 0; }
}
#define TEST(Suite, Name) \
static void Suite##_##Name(); \
static bool Suite##_##Name##_reg = [](){ testing::registry().push_back(Suite##_##Name); return true; }(); \
static void Suite##_##Name()
#define EXPECT_EQ(a,b) assert((a)==(b))
#define EXPECT_NE(a,b) assert((a)!=(b))
#define EXPECT_TRUE(a) assert(a)
#define EXPECT_FALSE(a) assert(!(a))
#endif
