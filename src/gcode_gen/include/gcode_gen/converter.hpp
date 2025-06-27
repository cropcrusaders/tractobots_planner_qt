#ifndef GCODE_GEN_CONVERTER_HPP
#define GCODE_GEN_CONVERTER_HPP

#include <string>
#include <string_view>
#include <vector>

#include "gcode_gen/waypoint.hpp"

namespace gcode_gen {

std::vector<std::string> toGcode(const std::vector<Waypoint>& wps,
                                 bool relative,
                                 int downCode,
                                 int upCode,
                                 double speed,
                                 std::string_view comment,
                                 bool comment_gps,
                                 int epsg,
                                 std::pair<double,double> origin);

} // namespace gcode_gen

#endif // GCODE_GEN_CONVERTER_HPP
