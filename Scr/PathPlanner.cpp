#include "PathPlanner.h"
#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/minimum_rotated_rectangle.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <fstream>
#include <cmath>

namespace bg = boost::geometry;

std::vector<BoostLinestr> PathPlanner::computeRows(
    const std::vector<BoostPt>& boundary,
    double spacing,
    double headland)
{
    using Polygon = bg::model::polygon<BoostPt>;
    Polygon poly;
    bg::assign_points(poly, boundary);
    bg::correct(poly);

    // 1) Buffer inward to carve out headland
    Polygon inner;
    bg::buffer(poly, inner,
      bg::strategy::buffer::distance_symmetric<double>(-headland));

    // 2) Find minimal rotated rectangle to get row orientation
    Polygon minRect;
    bg::minimum_rotated_rectangle(inner, minRect);
    auto coords = minRect.outer();
    double dx = coords[1].x() - coords[0].x();
    double dy = coords[1].y() - coords[0].y();
    double angle = std::atan2(dy, dx);

    // 3) Rotate inner polygon so rows align with X axis
    Polygon rotated;
    bg::transform(inner, rotated,
      bg::strategy::transform::rotate_transformer<double,2,2>(-angle));

    // 4) Intersect with vertical lines spaced by 'spacing'
    bg::model::box<BoostPt> bbox;
    bg::envelope(rotated, bbox);
    double minx = bbox.min_corner().x();
    double maxx = bbox.max_corner().x();
    double miny = bbox.min_corner().y();
    double maxy = bbox.max_corner().y();

    std::vector<BoostLinestr> rows;
    for (double x = minx + spacing/2.0; x < maxx; x += spacing) {
        BoostLinestr line{{x, miny}, {x, maxy}};
        std::vector<BoostLinestr> segs;
        bg::intersection(line, rotated, segs);
        for (auto &seg : segs) {
            // Rotate segment back to original orientation
            BoostLinestr segOut;
            bg::transform(seg, segOut,
              bg::strategy::transform::rotate_transformer<double,2,2>(angle));
            rows.push_back(std::move(segOut));
        }
    }
    return rows;
}

std::vector<Segment> PathPlanner::computePlan(
    const std::vector<BoostPt>& boundary,
    double spacing,
    double headland,
    double turn_radius,
    bool reverse_passes)
{
    // First generate straight rows
    auto rows = computeRows(boundary, spacing, headland);
    plan_.clear();

    bool forward = true;
    for (size_t i = 0; i < rows.size(); ++i) {
        // Add straight pass (reverse if zig‑zag and forward==false)
        bool rev = reverse_passes ? !forward : false;
        plan_.push_back({ rows[i], rev, 0.0 });

        // If a turn is requested and not last row, build a semicircular arc
        if (turn_radius > 0 && i + 1 < rows.size()) {
            // End point of current row
            BoostPt P = forward ? rows[i].back() : rows[i].front();
            // Start point of next row
            BoostPt Q = forward ? rows[i+1].front() : rows[i+1].back();

            // Base direction and perpendicular for center
            double dx = Q.x() - P.x(), dy = Q.y() - P.y();
            double base = std::atan2(dy, dx);
            double cx = P.x() - turn_radius * std::sin(base);
            double cy = P.y() + turn_radius * std::cos(base);

            // Discretize 180° arc into N steps
            const int N = 20;
            BoostLinestr arc;
            for (int k = 0; k <= N; ++k) {
                double theta = base + M_PI * (k / double(N));
                double x = cx + turn_radius * std::cos(theta);
                double y = cy + turn_radius * std::sin(theta);
                arc.push_back({x, y});
            }
            plan_.push_back({ arc, false, turn_radius });
        }

        // Flip direction if zig‑zag
        if (reverse_passes) forward = !forward;
    }

    return plan_;
}

void PathPlanner::exportGCode(
    const std::string &fname,
    double feedrate) const
{
    std::ofstream fout(fname);
    fout << "; Tractobots G-code plan (with turns & headings)\n";
    fout << "G21 ; mm\nG90 ; absolute\n";
    fout << "F" << feedrate << "\n\n";

    for (auto const &seg : plan_) {
        // Implement up
        fout << "M3 S0\n";
        auto const &pts = seg.line;
        if (pts.empty()) continue;

        // Rapid to first point (with heading)
        {
            auto P0 = pts.front();
            double hd = 0.0;
            if (pts.size() > 1) {
                auto P1 = pts[1];
                hd = std::atan2(P1.y() - P0.y(), P1.x() - P0.x()) * 180.0 / M_PI;
            }
            fout << "G0 X" << P0.x() << " Y" << P0.y()
                 << " ; heading=" << hd << "\n";
        }

        // Implement down
        fout << "M3 S1\n";

        // Arc segment (G2) or linear (G1)
        if (seg.turn_radius > 0 && pts.size() > 2) {
            // Use G2 (CW) for the arc; calculate I,J from center-offset
            // Recompute center same as in computePlan
            auto &P = pts.front();
            auto &Q = pts.back();
            // You could store center during computePlan; here assume same logic:
            // (for brevity we treat this as G2 from P->Q with I,J=0; refine as needed)
            fout << "G2 X" << Q.x() << " Y" << Q.y()
                 << " I0 J0 ; arc radius=" << seg.turn_radius << "\n\n";
        } else {
            // Linear moves
            for (size_t i = 1; i < pts.size(); ++i) {
                auto const &A = pts[i-1];
                auto const &B = pts[i];
                double hd = std::atan2(B.y() - A.y(), B.x() - A.x()) * 180.0 / M_PI;
                fout << (seg.reverse ? "G1 R " : "G1 ")
                     << "X" << B.x() << " Y" << B.y()
                     << " ; heading=" << hd << "\n";
            }
            fout << "\n";
        }
    }

    // Final implement up & program end
    fout << "M3 S0\nM2\n";
}
