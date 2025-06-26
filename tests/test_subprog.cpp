#include "gcode_gen/advanced_writer.hpp"
#include "gtest.h"
using namespace gcode_gen;

TEST(GCode, SubProgFold) {
    std::vector<Waypoint> w = {
        {0,0,0,true,true},{1,0,0,true,true},{1,0,0,false,true},
        {0,0,0,true,true},{1,0,0,true,true},{1,0,0,false,true},
        {0,0,0,true,true},{1,0,0,true,true},{1,0,0,false,true}
    };
    PlannerSettings ps; ps.use_subprograms=true;
    auto lines = AdvancedWriter(ps).write(w);
    bool hasO=false, hasCall=false;
    for(const auto& l:lines){ if(l=="O100") hasO=true; if(l.rfind("M98",0)==0) hasCall=true; }
    EXPECT_TRUE(hasO && hasCall);
}

