#ifndef GCODE_GEN_ADVANCED_WRITER_HPP
#define GCODE_GEN_ADVANCED_WRITER_HPP

#include "gcode_gen/writer.hpp"

namespace gcode_gen {

class AdvancedWriter : public Writer {
public:
    explicit AdvancedWriter(const PlannerSettings& s);
    std::vector<std::string> write(const std::vector<Waypoint>& wps) override;
protected:
    std::string write_move(double x,double y,double feed) override;
    std::string write_arc(double x,double y,double i,double j,double feed) override;
};

} // namespace gcode_gen

#endif
