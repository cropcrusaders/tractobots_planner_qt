#include "gcode_gen/converter.hpp"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace gcode_gen {

static constexpr double kDegToRad = M_PI / 180.0;
static constexpr double kEarthR = 6371000.0; // metres

std::vector<std::string> toGcode(const std::vector<Waypoint>& wp_in,
                                 bool relative,
                                 int downCode,
                                 int upCode,
                                 double speed,
                                 std::string_view comment) {
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
        lines.push_back(std::string("; ")+std::string(comment));
    lines.push_back(relative ? "G91" : "G90");

    bool any_tool = false;
    for (const auto& w : wps) if (w.has_tool) { any_tool = true; break; }
    bool tool_state = false;
    Waypoint prev = wps.front();
    (void)prev; // silence unused if only one
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
        std::ostringstream ss;
        ss.setf(std::ios::fixed); ss.precision(6);
        ss << "G1 ";
        if (relative) {
            const auto& prev_wp = (i==0?wps[0]:wps[i-1]);
            double dx = (wp.lon - prev_wp.lon) * std::cos((wp.lat+prev_wp.lat)/2*kDegToRad) * kDegToRad * kEarthR;
            double dy = (wp.lat - prev_wp.lat) * kDegToRad * kEarthR;
            ss << "X" << dx << " Y" << dy;
        } else {
            ss << "X" << wp.lat << " Y" << wp.lon;
        }
        if (i==0) {
            ss << " F" << speed;
        }
        lines.push_back(ss.str());
    }
    return lines;
}

} // namespace gcode_gen
