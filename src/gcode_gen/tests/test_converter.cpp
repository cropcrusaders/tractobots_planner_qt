#include "gcode_gen/converter.hpp"
#include "gcode_gen/waypoint.hpp"

#include <cassert>
#include <fstream>
#include <vector>

using namespace gcode_gen;

int main() {
    const char* data = "[{'lat':35.6896,'lon':-120.6915},{'lat':35.6897,'lon':-120.6916},{'lat':35.6898,'lon':-120.6917}]";
    std::ofstream f("test.json");
    std::string s(data);
    for (char& c : s) if (c=='\'') c='"';
    f << s;
    f.close();

    auto wps = readWaypoints("test.json", "alt");
    auto lines = toGcode(wps, false, 100, 101, 1.0, "demo");
    assert(!lines.empty());
    assert(lines[0].rfind(";",0)==0);
    assert(lines.size()>=3);
    assert(lines[2]=="G1 X35.689600 Y-120.691500 F1" || lines[2]=="G1 X35.689600 Y-120.691500" || lines[2].rfind("G1 X35.689600 Y-120.691500",0)==0);

    auto rel = toGcode(wps, true, 100, 101, 1.0, "demo");
    assert(rel.size()>=3);
    assert(rel[1]=="G91");
    return 0;
}
