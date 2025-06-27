#ifndef GCODE_GEN_GPS_TRANSFORMS_HPP
#define GCODE_GEN_GPS_TRANSFORMS_HPP

#include <utility>

namespace gcode_gen {

struct UTMPoint {
    double easting;
    double northing;
    int zone;
    bool northp; // true if northern hemisphere
};

struct WGS84Point {
    double lat;
    double lon;
};

// Convert WGS84 latitude/longitude in degrees to UTM coordinates
UTMPoint wgs84ToUTM(double lat, double lon, int epsg = 0);

// Convert UTM coordinates back to WGS84 latitude/longitude in degrees
WGS84Point utmToWgs84(double easting, double northing, int zone, bool northp);

} // namespace gcode_gen

#endif // GCODE_GEN_GPS_TRANSFORMS_HPP
