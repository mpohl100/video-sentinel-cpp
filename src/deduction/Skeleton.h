#pragma once

#include "math2d/math2d.h"

#include <vector>

namespace deduct {

struct Skeleton{
    std::vector<math2d::Line> skeleton;
};

} // namespace deduct