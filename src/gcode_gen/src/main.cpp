#include "gcode_gen/converter.hpp"
#include "gcode_gen/waypoint.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <limits>
#include <cmath>

using namespace gcode_gen;

int main(int argc, char** argv) {
    std::string in_path;
    std::string out_path;
    bool relative = false;
    int down_code = 100;
    int up_code = 101;
    double speed = 1.0;
    std::string comment;
    std::string z_field = "alt";
    int epsg = 0;
    bool comment_gps = false;
    std::pair<double,double> origin{std::numeric_limits<double>::quiet_NaN(),
                                    std::numeric_limits<double>::quiet_NaN()};

    for (int i=1;i<argc;i++) {
        std::string arg = argv[i];
        auto get_val = [&](const std::string& name)->std::string {
            if (i+1 >= argc) throw std::runtime_error("Missing value for " + name);
            return argv[++i];
        };
        if (arg == "--in" || arg == "--waypoints") in_path = get_val(arg);
        else if (arg == "--out") out_path = get_val("--out");
        else if (arg == "--relative") relative = true;
        else if (arg == "--down-code") down_code = std::stoi(get_val(arg));
        else if (arg == "--up-code") up_code = std::stoi(get_val(arg));
        else if (arg == "--speed") speed = std::stod(get_val(arg));
        else if (arg == "--comment") comment = get_val(arg);
        else if (arg == "--z-field") z_field = get_val(arg);
        else if (arg == "--epsg") epsg = std::stoi(get_val(arg));
        else if (arg == "--origin") {
            auto v = get_val(arg);
            auto comma = v.find(',');
            if (comma == std::string::npos) throw std::runtime_error("--origin expects lat,lon");
            origin.first = std::stod(v.substr(0, comma));
            origin.second = std::stod(v.substr(comma+1));
        }
        else if (arg == "--comment-gps") comment_gps = true;
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if (in_path.empty() || out_path.empty()) {
        std::cerr << "--in and --out required\n";
        return 1;
    }

    auto wps = readWaypoints(in_path, z_field);
    if (wps.size() > 10000) {
        std::cerr << "Warning: large waypoint count: " << wps.size() << "\n";
    }
    if (std::isnan(origin.first)) {
        origin.first = wps.front().lat;
        origin.second = wps.front().lon;
    }

    auto lines = toGcode(wps, relative, down_code, up_code, speed, comment,
                         comment_gps, epsg, origin);

    std::ofstream out(out_path);
    if (!out.is_open()) {
        std::cerr << "Failed to open output: " << out_path << "\n";
        return 1;
    }
    for (const auto& l : lines) out << l << "\n";
    return 0;
}
