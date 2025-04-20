#pragma once
#include <vector>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

using BoostPt      = boost::geometry::model::d2::point_xy<double>;
using BoostLinestr = boost::geometry::model::linestring<BoostPt>;

class PathPlanner {
public:
    /** boundary: clicked points in local meters **/
    std::vector<BoostLinestr> computeRows(
        const std::vector<BoostPt>& boundary,
        double spacing, double headland);

    /** write CNC‑style G‑code to disk **/
    void exportGCode(const std::string &filename,
                     double feedrate=1000.0);
private:
    std::vector<BoostLinestr> rows_;
};
