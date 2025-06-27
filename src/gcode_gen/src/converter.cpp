#include "gcode_gen/converter.hpp"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <sstream>
#include <stdexcept>
#include "gcode_gen/gps_transforms.hpp"

namespace gcode_gen {

static constexpr double kDegToRad = M_PI / 180.0;

std::vector<std::string> toGcode(const std::vector<Waypoint>& wp_in,
                                 bool relative,
                                 int downCode,
                                 int upCode,
                                 double speed,
                                 std::string_view comment,
                                 bool comment_gps,
                                 int epsg,
                                 std::pair<double,double> origin) {
    if (wp_in.empty()) return {};

    // copy while removing consecutive duplicates
    std::vector<Waypoint> wps;
    wps.reserve(wp_in.size());
    Waypoint last{9999,9999,0,false};
    for (const auto& w : wp_in) {
        if (w.lat==last.lat && w.lon==last.lon && w.alt==last.alt && w.tool_down==last.tool_down) continue;
        wps.push_back(w);
        last = w;
    }

    std::vector<std::string> lines;
    if (!comment.empty())
        lines.push_back(std::string("; ") + std::string(comment));

    auto origin_utm = wgs84ToUTM(origin.first, origin.second, epsg);
    std::ostringstream hdr;
    hdr.setf(std::ios::fixed); hdr.precision(5);
    hdr << "; (Origin WGS84: " << origin.first << "\xC2\xB0, " << origin.second
        << "\xC2\xB0  EPSG:" << epsg
        << "  False Northing: " << (origin_utm.northp ? 0 : 10000000) << ")";
    lines.push_back(hdr.str());
    lines.push_back(relative ? "G91" : "G90");

    bool any_tool = false;
    for (const auto& w : wps) if (w.has_tool) { any_tool = true; break; }
    bool tool_state = false;
    double last_x = 0.0, last_y = 0.0;
    for (size_t i=0;i<wps.size();++i) {
        const auto& wp = wps[i];
        if (any_tool) {
            if (i==0) {
                tool_state = wp.tool_down;
                lines.push_back(std::string("M") + std::to_string(tool_state ? downCode : upCode));
            } else if (wp.tool_down != tool_state) {
                tool_state = wp.tool_down;
                lines.push_back(std::string("M") + std::to_string(tool_state ? downCode : upCode));
            }
        }

        auto utm = wgs84ToUTM(wp.lat, wp.lon, epsg);
        double x = utm.easting - origin_utm.easting;
        double y = utm.northing - origin_utm.northing;

        double dx = x - last_x;
        double dy = y - last_y;
        double gap = std::hypot(dx, dy);

        if (gap > 3.0)
            lines.push_back("M200 (section off)");

        std::ostringstream ss;
        ss.setf(std::ios::fixed); ss.precision(3);
        ss << "G1 ";
        if (relative) {
            ss << "X" << dx << " Y" << dy;
            last_x += dx; last_y += dy;
        } else {
            ss << "X" << x << " Y" << y;
            last_x = x; last_y = y;
        }
        if (i==0) ss << " F" << speed;
        if (comment_gps) {
            ss << "   ; gps=" << wp.lat << "," << wp.lon;
        }
        lines.push_back(ss.str());
        if (gap > 3.0)
            lines.push_back("M201");
    }
    return lines;
}

} // namespace gcode_gen
