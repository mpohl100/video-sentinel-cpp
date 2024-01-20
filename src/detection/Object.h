#pragma once

#include "Slices.h"

#include "matrix/Matrix.h"

#include "Rectangle.h"

#include <memory>
#include <vector>

namespace od {

struct Object {
  Object() = default;
  Object(const Object &) = default;
  Object(Object &&) = default;
  Object &operator=(const Object &) = default;
  Object &operator=(Object &&) = default;
  Object(const Slices& slices) : slices{slices} {}
  Slices slices;
};

struct ObjectsPerRectangle {

  std::vector<Object> objects;
  Rectangle rectangle;
};

struct AllObjects {
  matrix::Matrix<ObjectsPerRectangle> objects_per_rectangle;
};

} // namespace od