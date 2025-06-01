#include "gcode_gen/converter.hpp"
#include "gcode_gen/waypoint.hpp"

#include <fstream>
#include <iostream>
#include <string>

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

    for (int i=1;i<argc;i++) {
        std::string arg = argv[i];
        auto get_val = [&](const std::string& name)->std::string {
            if (i+1 >= argc) throw std::runtime_error("Missing value for " + name);
            return argv[++i];
        };
        if (arg == "--in") in_path = get_val("--in");
        else if (arg == "--out") out_path = get_val("--out");
        else if (arg == "--relative") relative = true;
        else if (arg == "--down-code") down_code = std::stoi(get_val(arg));
        else if (arg == "--up-code") up_code = std::stoi(get_val(arg));
        else if (arg == "--speed") speed = std::stod(get_val(arg));
        else if (arg == "--comment") comment = get_val(arg);
        else if (arg == "--z-field") z_field = get_val(arg);
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
    auto lines = toGcode(wps, relative, down_code, up_code, speed, comment);

    std::ofstream out(out_path);
    if (!out.is_open()) {
        std::cerr << "Failed to open output: " << out_path << "\n";
        return 1;
    }
    for (const auto& l : lines) out << l << "\n";
    return 0;
}
