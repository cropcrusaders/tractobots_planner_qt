#pragma once

#include <vector>
#include <string>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>

namespace bg = boost::geometry;
using BoostPt      = bg::model::d2::point_xy<double>;
using BoostLinestr = bg::model::linestring<BoostPt>;

/** A single straight pass or arc or reverse segment **/
struct Segment {
  BoostLinestr line;       ///< points to follow
  bool         reverse;    ///< drive backwards if true
  double       turn_radius;///< >0 for arc, 0 for straight or reverse
};

class PathPlanner {
public:
  PathPlanner() = default;

  /// Just compute straight rows (no turns or reverse)
  std::vector<BoostLinestr> computeRows(
    const std::vector<BoostPt>& boundary,
    double spacing,
    double headland);

  /**
   * Compute full plan:
   *  - straight passes (zig‑zag if reverse_passes)
   *  - optional reverse segment at end of each pass
   *  - smoothing arc turn of radius turn_radius
   *
   * @param boundary       field polygon
   * @param spacing        row spacing (m)
   * @param headland       inward buffer (m)
   * @param turn_radius    turn smoothing radius (m; 0 = sharp)
   * @param reverse_passes zig‑zag direction if true
   * @param reverse_dist   how far to reverse at each end (m; 0 = none)
   */
  std::vector<Segment> computePlan(
    const std::vector<BoostPt>& boundary,
    double spacing,
    double headland,
    double turn_radius,
    bool   reverse_passes,
    double reverse_dist);

  /**
   * Export last plan to G‑code, embedding heading comments:
   *  M3 S0/S1 = implement up/down
   *  G0 = rapid, G1 = linear, G2 = CW arc, G3 = CCW arc
   */
  void exportGCode(
    const std::string &fname,
    double feedrate = 1000.0) const;

private:
  std::vector<Segment> plan_;
};
