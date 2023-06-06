#pragma once

#include <cmath>
inline const double EPSILON = 1e-6;

namespace tr_cat {
namespace geo {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return std::abs(lat - other.lat) < EPSILON && std::abs(lng - other.lng) < EPSILON;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    static const int earth_rad = 6371000;

    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * earth_rad;
}

}//geo
}//tr_cat
