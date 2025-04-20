#pragma once

#include <vector>
#include <string>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>

namespace bg = boost::geometry;

// 2D point and polyline types
using BoostPt      = bg::model::d2::point_xy<double>;
using BoostLinestr = bg::model::linestring<BoostPt>;

/**
 * A single segment in the plan: either a straight pass or a turn arc.
 */
struct Segment {
  BoostLinestr line;       ///< Sequence of points to follow
  bool         reverse;    ///< true = drive backwards
  double       turn_radius;///< >0 for an arc segment (G2/G3), 0 for straight
};

class PathPlanner {
public:
  PathPlanner() = default;

  /**
   * Compute the polygon’s rows (without turns).
   * @param boundary   CCW points defining field (local coords)
   * @param spacing    distance between adjacent rows (m)
   * @param headland   inward buffer margin where no passes go (m)
   * @return list of straight lines for each row
   */
  std::vector<BoostLinestr> computeRows(
    const std::vector<BoostPt>& boundary,
    double spacing,
    double headland);

  /**
   * Compute the full pass‑by‑pass plan, including turn arcs.
   * @param boundary        field polygon
   * @param spacing         row spacing
   * @param headland        buffer margin
   * @param turn_radius     radius for smoothing turns (m; 0 = sharp)
   * @param reverse_passes  if true, alternate drive direction each row
   * @return ordered segments (rows + turn arcs)
   */
  std::vector<Segment> computePlan(
    const std::vector<BoostPt>& boundary,
    double spacing,
    double headland,
    double turn_radius,
    bool   reverse_passes);

  /**
   * Export the last plan to a G‑code file.
   * - M3 S0/S1 = implement up/down
   * - G0 = rapid move
   * - G1 = linear cut
   * - G2 = CW arc, G3 = CCW arc
   * Each move line is annotated with “; heading=…” in degrees.
   */
  void exportGCode(
    const std::string &fname,
    double feedrate = 1000.0) const;

private:
  std::vector<Segment> plan_;  ///< holds the computed plan
};
