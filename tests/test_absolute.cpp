#include "gcode_gen/writer.hpp"
#include <string>
#include "gtest.h"
using namespace gcode_gen;

TEST(GCode, IncrementalVsAbsolute) {
    std::vector<Waypoint> w{{0,0,0},{1,1,0}};
    PlannerSettings abs_s; abs_s.incremental=false;
    PlannerSettings rel_s; rel_s.incremental=true;
    auto abs_lines = Writer(abs_s).write(w);
    auto rel_lines = Writer(rel_s).write(w);
    EXPECT_NE(abs_lines[0].find("G90"), std::string::npos);
    EXPECT_NE(rel_lines[0].find("G91"), std::string::npos);
}

