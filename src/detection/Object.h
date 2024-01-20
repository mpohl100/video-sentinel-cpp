#pragma once

#include "matrix/Matrix.h"

#include "Rectangle.h"

#include <vector>

namespace od {

struct Object {};

struct ObjectsPerRectangle {

  std::vector<Object> objects;
  Rectangle rectangle;
};

struct AllObjects {
  matrix::Matrix<ObjectsPerRectangle> objects_per_rectangle;
};

} // namespace od