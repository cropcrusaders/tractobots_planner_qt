#include "PathPlanner.h"
#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/rotate.hpp>
#include <fstream>

std::vector<BoostLinestr> PathPlanner::computeRows(
    const std::vector<BoostPt> &boundary,
    double spacing, double headland)
{
    using namespace boost::geometry;
    model::polygon<BoostPt> poly(boundary);
    // Optionally buffer for headland
    model::polygon<BoostPt> inner;
    buffer(poly, inner,
           strategy::buffer::distance_symmetric<double>(-headland));
    // ... compute oriented bounding box, rotate, generate linestrings,
    // ... intersect with inner, rotate back, store in rows_ ...
    rows_.clear();
    // (Detailed boost::geometry code omitted for brevity)
    return rows_;
}

void PathPlanner::exportGCode(
    const std::string &fname, double feedrate)
{
    std::ofstream fout(fname);
    fout << "; Tractobots G-code\n";
    fout << "G21\nG90\nF" << feedrate << "\n\n";
    for (auto &row : rows_) {
        // up implement
        fout << "M3 S0\n";
        auto &P0 = row.front(), &P1 = row.back();
        fout << "G0 X" << P0.x() << " Y" << P0.y() << "\n";
        fout << "M3 S1\n";
        for (auto &pt : row) {
            fout << "G1 X" << pt.x() << " Y" << pt.y() << "\n";
        }
        fout << "\n";
    }
    fout << "M3 S0\nM2\n";
}
