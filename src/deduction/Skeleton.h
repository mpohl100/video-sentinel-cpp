#pragma once

#include "math2d/math2d.h"
#include "math2d/coordinated.h"

#include <string>
#include <vector>

namespace deduct {

struct Skeleton{
    struct Area{
        math2d::CoordinateSystem coordinate_system;
        math2d::number_type radius;

        std::string to_string() const {
            return "Area{coordinate_system: " + coordinate_system.to_string() + "; radius: " + std::to_string(radius) + "}";
        }
    };

    std::vector<Area> areas;
};

} // namespace deduct