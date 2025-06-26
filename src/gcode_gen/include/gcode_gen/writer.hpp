#ifndef GCODE_GEN_WRITER_HPP
#define GCODE_GEN_WRITER_HPP

#include <string>
#include <vector>
#include "gcode_gen/waypoint.hpp"

namespace gcode_gen {

struct PlannerSettings {
    bool incremental = false;
    double feed_work = 1000.0;
    double feed_turn = 800.0;
    double feed_reverse = 600.0;
    bool use_subprograms = true;
    double safe_z = 5.0;
    double work_z = 0.0;
    int coord_system = 54; // G54..G59
    double offset_x = 0.0;
    double offset_y = 0.0;
    std::string plane = "G17"; // G17/G18
    std::string field_name;
};

class Writer {
public:
    explicit Writer(const PlannerSettings& settings);
    virtual ~Writer() = default;

    virtual std::vector<std::string> write(const std::vector<Waypoint>& wps);

protected:
    virtual std::string write_header();
    virtual std::string write_footer();
    virtual std::string write_move(double x, double y, double feed);
    virtual std::string write_arc(double x, double y, double i, double j, double feed);

    PlannerSettings ps_;
    double last_x_ = 0.0;
    double last_y_ = 0.0;
    bool first_move_ = true;
};

} // namespace gcode_gen

#endif
