#pragma once

#include "Rectangle.h"

#include "opencv2/core/mat.hpp"

#include <vector>

namespace od {
struct AllRectangles {
  std::vector<Rectangle> rectangles;
};

void establishing_shot_slices(AllRectangles &ret, const cv::Mat &contours,
                              const Rectangle &rectangle);
} // namespace od