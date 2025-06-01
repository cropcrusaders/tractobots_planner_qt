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
    double headland,
    double heading_deg)
{
    using Polygon = bg::model::polygon<BoostPt>;
    Polygon poly; bg::assign_points(poly, boundary); bg::correct(poly);

    // carve off headland
    Polygon inner;
    bg::buffer(poly, inner,
      bg::strategy::buffer::distance_symmetric<double>(-headland));

    double angle;
    if (std::isnan(heading_deg)) {
        // figure row orientation automatically
        Polygon minRect;
        bg::minimum_rotated_rectangle(inner, minRect);
        auto coords = minRect.outer();
        double dx = coords[1].x() - coords[0].x();
        double dy = coords[1].y() - coords[0].y();
        angle = std::atan2(dy, dx);
    } else {
        angle = heading_deg * M_PI / 180.0;
    }

    // rotate so rows align with X
    Polygon rotated;
    bg::transform(inner, rotated,
      bg::strategy::transform::rotate_transformer<double,2,2>(-angle));

    // intersect with vertical lines
    bg::model::box<BoostPt> bbox; bg::envelope(rotated, bbox);
    double minx=bbox.min_corner().x(), maxx=bbox.max_corner().x();
    double miny=bbox.min_corner().y(), maxy=bbox.max_corner().y();

    std::vector<BoostLinestr> rows;
    for(double x=minx+spacing/2; x<maxx; x+=spacing){
        BoostLinestr L{{x,miny},{x,maxy}};
        std::vector<BoostLinestr> segs;
        bg::intersection(L, rotated, segs);
        for(auto &s:segs){
            BoostLinestr out;
            bg::transform(s, out,
              bg::strategy::transform::rotate_transformer<double,2,2>(angle));
            rows.push_back(std::move(out));
        }
    }
    return rows;
}

std::vector<Segment> PathPlanner::computePlan(
    const std::vector<BoostPt>& boundary,
    double spacing,
    double headland,
    double heading_deg,
    double turn_radius,
    bool   reverse_passes,
    double reverse_dist)
{
    auto rows = computeRows(boundary, spacing, headland, heading_deg);
    plan_.clear();
    bool forward = true;

    for(size_t i=0; i<rows.size(); ++i){
      // 1) Straight pass
      bool rev = reverse_passes ? !forward : false;
      plan_.push_back({ rows[i], rev, 0.0 });

      // 2) Reverse back if requested
      if(reverse_dist>0){
        // end point of this pass
        BoostPt P = forward ? rows[i].back() : rows[i].front();
        // compute heading direction of pass
        BoostPt Pprev = forward
          ? rows[i][ rows[i].size()-2 ]
          : rows[i][1];
        double hd = std::atan2(P.y()-Pprev.y(), P.x()-Pprev.x());
        // compute back‑up point
        BoostPt B( P.x() - reverse_dist*std::cos(hd),
                   P.y() - reverse_dist*std::sin(hd) );
        // build reverse segment (two points)
        BoostLinestr revLine;
        revLine.push_back(P);
        revLine.push_back(B);
        plan_.push_back({ revLine, true, 0.0 });
      }

      // 3) Turn arc if requested
      if(turn_radius>0 && i+1<rows.size()){
        auto &r = rows[i];
        auto &n = rows[i+1];
        // pivot points
        BoostPt P = forward ? r.back()  : r.front();
        BoostPt Q = forward ? n.front() : n.back();
        // direction & center
        double dx = Q.x()-P.x(), dy = Q.y()-P.y();
        double base = std::atan2(dy, dx);
        double cx = P.x() - turn_radius*std::sin(base);
        double cy = P.y() + turn_radius*std::cos(base);
        // discretize 180° arc
        BoostLinestr arc;
        const int N=20;
        for(int k=0;k<=N;++k){
          double theta = base + M_PI*(k/double(N));
          arc.push_back({
            cx + turn_radius*std::cos(theta),
            cy + turn_radius*std::sin(theta)
          });
        }
        plan_.push_back({ arc, false, turn_radius });
      }

      // 4) flip for zig‑zag
      if(reverse_passes) forward = !forward;
    }

    return plan_;
}

void PathPlanner::exportGCode(
    const std::string &fname,
    double feedrate) const
{
    std::ofstream fout(fname);
    fout<<"; Tractobots G‑code plan (with reverse & turns)\n";
    fout<<"G21 ; mm\nG90 ; absolute\n";
    fout<<"F"<<feedrate<<"\n\n";

    for(auto const &seg:plan_){
      auto &pts = seg.line;
      if(pts.empty()) continue;
      // up implement
      fout<<"M3 S0\n";
      // rapid to start w/ heading
      {
        auto P0=pts.front(), P1=pts.size()>1?pts[1]:pts.front();
        double hd=std::atan2(P1.y()-P0.y(),P1.x()-P0.x())*180.0/M_PI;
        fout<<"G0 X"<<P0.x()<<" Y"<<P0.y()<<" ; heading="<<hd<<"\n";
      }
      // down implement
      fout<<"M3 S1\n";

      // choose arc or line
      if(seg.turn_radius>0 && pts.size()>2){
        // simple G2 w/ zero I/J placeholder
        auto Pend=pts.back();
        fout<<"G2 X"<<Pend.x()<<" Y"<<Pend.y()
            <<" I0 J0 ; arc r="<<seg.turn_radius<<"\n\n";
      } else {
        // straight or reverse
        for(size_t i=1;i<pts.size();++i){
          auto &A=pts[i-1],&B=pts[i];
          double hd=std::atan2(B.y()-A.y(),B.x()-A.x())*180.0/M_PI;
          fout<<(seg.reverse?"G1 R ":"G1 ")
              <<"X"<<B.x()<<" Y"<<B.y()
              <<" ; heading="<<hd<<"\n";
        }
        fout<<"\n";
      }
    }
    fout<<"M3 S0\nM2\n";
}
