#include "gcode_gen/converter.hpp"
#include "gcode_gen/waypoint.hpp"
#include "gcode_gen/writer.hpp"
#include "gcode_gen/advanced_writer.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace gcode_gen;

int main(int argc, char** argv) {
    std::string in_path;
    std::string out_path;
    PlannerSettings ps;
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
        else if (arg == "--incremental") ps.incremental = true;
        else if (arg == "--feed-work") ps.feed_work = std::stod(get_val(arg));
        else if (arg == "--feed-turn") ps.feed_turn = std::stod(get_val(arg));
        else if (arg == "--feed-rev") ps.feed_reverse = std::stod(get_val(arg));
        else if (arg == "--comment") comment = get_val(arg);
        else if (arg == "--z-field") z_field = get_val(arg);
        else if (arg == "--no-subprog") ps.use_subprograms = false;
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
    auto lines = AdvancedWriter(ps).write(wps);

    std::ofstream out(out_path);
    if (!out.is_open()) {
        std::cerr << "Failed to open output: " << out_path << "\n";
        return 1;
    }
    for (const auto& l : lines) out << l << "\n";
    return 0;
}
