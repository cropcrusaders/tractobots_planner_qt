#ifndef GCODE_GEN_WAYPOINT_HPP
#define GCODE_GEN_WAYPOINT_HPP
#include <string>
#include <vector>

namespace gcode_gen {

struct Waypoint {
    double lat{0};
    double lon{0};
    double alt{0};
    bool tool_down{false};
    bool has_tool{false};
};

std::vector<Waypoint> readWaypoints(const std::string& path, const std::string& z_field = "alt");

} // namespace gcode_gen

#endif // GCODE_GEN_WAYPOINT_HPP
