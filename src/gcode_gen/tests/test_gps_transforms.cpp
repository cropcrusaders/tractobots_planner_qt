#include "gcode_gen/gps_transforms.hpp"
#include <cassert>
#include <cmath>

using namespace gcode_gen;

int main(){
    double lat = -27.4698; // Brisbane
    double lon = 153.0251;
    int epsg = 32756;
    auto utm = wgs84ToUTM(lat, lon, epsg);
    auto ll = utmToWgs84(utm.easting, utm.northing, utm.zone, utm.northp);
    assert(std::abs(ll.lat - lat) < 1e-7);
    assert(std::abs(ll.lon - lon) < 1e-7);

    UTMPoint a{100.0, (utm.northp?0:10000000.0)+200.0, utm.zone, utm.northp};
    auto la = utmToWgs84(a.easting,a.northing,a.zone,a.northp);
    auto a2 = wgs84ToUTM(la.lat, la.lon, epsg);
    UTMPoint b{150.0, (utm.northp?0:10000000.0)+250.0, utm.zone, utm.northp};
    auto lb = utmToWgs84(b.easting,b.northing,b.zone,b.northp);
    auto b2 = wgs84ToUTM(lb.lat, lb.lon, epsg);
    double dx1 = b.easting - a.easting;
    double dy1 = b.northing - a.northing;
    double dx2 = b2.easting - a2.easting;
    double dy2 = b2.northing - a2.northing;
    assert(std::abs(dx1 - dx2) < 1e-6);
    assert(std::abs(dy1 - dy2) < 1e-6);
    return 0;
}
