#include "PathPlanner.h"
#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/minimum_rotated_rectangle.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>
#include <fstream>
#include <cmath>

namespace bg = boost::geometry;

std::vector<BoostLinestr> PathPlanner::computeRows(
    const std::vector<BoostPt> &boundary,
    double spacing, double headland)
{
    using Polygon = bg::model::polygon<BoostPt>;
    Polygon poly;
    bg::assign_points(poly, boundary);
    bg::correct(poly);

    // 1) Buffer inward to remove headland
    Polygon inner;
    bg::buffer(poly, inner,
      bg::strategy::buffer::distance_symmetric<double>(-headland));

    // 2) Compute min‑area rectangle & its orientation
    Polygon minRect;
    bg::minimum_rotated_rectangle(inner, minRect);
    auto coords = minRect.outer();
    double dx = coords[1].x() - coords[0].x();
    double dy = coords[1].y() - coords[0].y();
    double angle = std::atan2(dy, dx);

    // 3) Rotate inner polygon so rows align with X axis
    std::vector<Polygon> rotated_in, rotated_out;
    rotated_in.push_back(inner);
    bg::transform(rotated_in, rotated_out,
      bg::strategy::transform::rotate_transformer<double,2,2>(
         -angle));

    // 4) Generate vertical lines and intersect
    bg::model::box<BoostPt> bbox;
    bg::envelope(rotated_out.front(), bbox);
    double minx = bbox.min_corner().x();
    double maxx = bbox.max_corner().x();
    double miny = bbox.min_corner().y();
    double maxy = bbox.max_corner().y();

    rows_.clear();
    for (double x = minx + spacing/2.0; x < maxx; x += spacing) {
        BoostLinestr line{{x, miny}, {x, maxy}};
        std::vector<BoostLinestr> segs;
        bg::intersection(line, rotated_out.front(), segs);
        for (auto &seg : segs) {
            // rotate segment back to original orientation
            BoostLinestr out;
            bg::transform(seg, out,
              bg::strategy::transform::rotate_transformer<double,2,2>(
                 angle));
            rows_.push_back(out);
        }
    }
    return rows_;
}

void PathPlanner::exportGCode(
    const std::string &fname, double feedrate)
{
    std::ofstream fout(fname);
    fout << "; Tractobots G-code plan\n";
    fout << "G21 ; mm\nG90 ; absolute\n";
    fout << "F" << feedrate << "\n\n";

    for (auto &row : rows_) {
        // implement up
        fout << "M3 S0\n";
        auto P0 = row.front();
        fout << "G0 X" << P0.x() << " Y" << P0.y() << "\n";
        // implement down
        fout << "M3 S1\n";
        for (auto &pt : row) {
            fout << "G1 X" << pt.x() << " Y" << pt.y() << "\n";
        }
        fout << "\n";
    }
    fout << "M3 S0\nM2\n";
}
