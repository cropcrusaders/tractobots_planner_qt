#include "gcode_gen/gps_transforms.hpp"
#include <cmath>

namespace gcode_gen {

static constexpr double kA = 6378137.0;                 // WGS84 major axis
static constexpr double kF = 1.0 / 298.257223563;       // flattening
static constexpr double kK0 = 0.9996;                   // scale factor

static inline int inferZone(double lon) {
    return int(std::floor((lon + 180.0) / 6.0)) + 1;
}

UTMPoint wgs84ToUTM(double lat, double lon, int epsg) {
    int zone;
    bool northp;
    if (epsg >= 32600 && epsg < 33000) {
        zone = epsg % 100;
        northp = (epsg < 32700);
    } else {
        zone = inferZone(lon);
        northp = lat >= 0.0;
        if (epsg != 0) zone = epsg % 100; // allow custom zone
    }

    double lat_rad = lat * M_PI / 180.0;
    double lon_rad = lon * M_PI / 180.0;

    double lon0 = ((zone - 1) * 6 - 180 + 3) * M_PI / 180.0; // central meridian

    double e2 = kF * (2 - kF);
    double ep2 = e2 / (1 - e2);

    double N = kA / std::sqrt(1 - e2 * std::sin(lat_rad) * std::sin(lat_rad));
    double T = std::tan(lat_rad) * std::tan(lat_rad);
    double C = ep2 * std::cos(lat_rad) * std::cos(lat_rad);
    double A = std::cos(lat_rad) * (lon_rad - lon0);

    double M = kA * ((1 - e2 / 4 - 3 * e2 * e2 / 64 - 5 * e2 * e2 * e2 / 256) * lat_rad
           - (3 * e2 / 8 + 3 * e2 * e2 / 32 + 45 * e2 * e2 * e2 / 1024) * std::sin(2 * lat_rad)
           + (15 * e2 * e2 / 256 + 45 * e2 * e2 * e2 / 1024) * std::sin(4 * lat_rad)
           - (35 * e2 * e2 * e2 / 3072) * std::sin(6 * lat_rad));

    double x = kK0 * N * (A + (1 - T + C) * A * A * A / 6
                + (5 - 18 * T + T * T + 72 * C - 58 * ep2) * A * A * A * A * A / 120)
                + 500000.0;

    double y = kK0 * (M + N * std::tan(lat_rad) * (A * A / 2
                + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24
                + (61 - 58 * T + T * T + 600 * C - 330 * ep2) * A * A * A * A * A * A / 720));

    if (!northp) y += 10000000.0;

    return {x, y, zone, northp};
}

WGS84Point utmToWgs84(double easting, double northing, int zone, bool northp) {
    double e2 = kF * (2 - kF);
    double ep2 = e2 / (1 - e2);
    double lon0 = ((zone - 1) * 6 - 180 + 3) * M_PI / 180.0;
    double x = easting - 500000.0;
    double y = northing;
    if (!northp) y -= 10000000.0;
    double M = y / kK0;

    double mu = M / (kA * (1 - e2 / 4 - 3 * e2 * e2 / 64 - 5 * e2 * e2 * e2 / 256));
    double e1 = (1 - std::sqrt(1 - e2)) / (1 + std::sqrt(1 - e2));

    double J1 = 3 * e1 / 2 - 27 * e1 * e1 * e1 / 32;
    double J2 = 21 * e1 * e1 / 16 - 55 * e1 * e1 * e1 * e1 / 32;
    double J3 = 151 * e1 * e1 * e1 / 96;
    double J4 = 1097 * e1 * e1 * e1 * e1 / 512;

    double fp = mu + J1 * std::sin(2 * mu) + J2 * std::sin(4 * mu) + J3 * std::sin(6 * mu) + J4 * std::sin(8 * mu);

    double C1 = ep2 * std::cos(fp) * std::cos(fp);
    double T1 = std::tan(fp) * std::tan(fp);
    double N1 = kA / std::sqrt(1 - e2 * std::sin(fp) * std::sin(fp));
    double R1 = N1 * (1 - e2) / (1 - e2 * std::sin(fp) * std::sin(fp));
    double D = x / (N1 * kK0);

    double lat = fp - (N1 * std::tan(fp) / R1) * (D * D / 2
            - (5 + 3 * T1 + 10 * C1 - 4 * C1 * C1 - 9 * ep2) * D * D * D * D / 24
            + (61 + 90 * T1 + 298 * C1 + 45 * T1 * T1 - 252 * ep2 - 3 * C1 * C1) * D * D * D * D * D * D / 720);
    lat = lat * 180.0 / M_PI;

    double lon = (D - (1 + 2 * T1 + C1) * D * D * D / 6
            + (5 - 2 * C1 + 28 * T1 - 3 * C1 * C1 + 8 * ep2 + 24 * T1 * T1) * D * D * D * D * D / 120) / std::cos(fp);
    lon = lon0 + lon;
    lon = lon * 180.0 / M_PI;

    return {lat, lon};
}

} // namespace gcode_gen
