#include "gcode_gen/advanced_writer.hpp"
#include <sstream>

namespace gcode_gen {

AdvancedWriter::AdvancedWriter(const PlannerSettings& s) : Writer(s) {}

std::vector<std::string> AdvancedWriter::write(const std::vector<Waypoint>& wps) {
    if (!ps_.use_subprograms) {
        return Writer::write(wps);
    }
    // naive subprogram detection: if >=3 rows identical
    std::vector<std::string> base = Writer::write(wps);
    // find indices of M3 S1 (start of row)
    std::vector<size_t> starts;
    for(size_t i=0;i<base.size();++i) {
        if(base[i]=="M3 S1") starts.push_back(i);
    }
    if(starts.size()<3) return base;
    size_t row_len = (starts.size()>1 ? starts[1] : base.size()) - starts[0];

    std::vector<std::string> out;
    out.reserve(base.size());
    // header before first row
    for(size_t i=0;i<starts[0];++i) out.push_back(base[i]);
    out.push_back("O100");
    for(size_t j=0;j<row_len;++j)
        out.push_back(base[starts[0]+j]);
    out.push_back("M99");
    out.push_back(";");
    std::ostringstream call; call << "M98 P100 L" << starts.size();
    out.push_back(call.str());
    // footer lines (after last row)
    for(size_t i=starts.back()+row_len;i<base.size();++i)
        out.push_back(base[i]);
    return out;
}

std::string AdvancedWriter::write_move(double x,double y,double feed){
    return Writer::write_move(x,y,feed);
}
std::string AdvancedWriter::write_arc(double x,double y,double i,double j,double feed){
    return Writer::write_arc(x,y,i,j,feed);
}

} // namespace gcode_gen
