#pragma once

#include "math2d/math2d.h"
#include "math2d/coordinated.h"

#include <vector>

namespace deduct {

struct Skeleton{
    struct Area{
        math2d::CoordinateSystem coordinate_system;
        math2d::number_type radius;
    };

    std::vector<Area> areas;
};

} // namespace deduct