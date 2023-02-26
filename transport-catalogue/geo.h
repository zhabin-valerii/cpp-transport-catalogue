#pragma once

namespace geo {

    struct Coordinates {
        double lat = 0.0; // ������
        double lng = 0.0; // �������
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo