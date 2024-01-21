#pragma once

#include "Slices.h"

#include "matrix/Matrix.h"

#include "Rectangle.h"

#include <memory>
#include <stdexcept>
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
  void append_right(const ObjectsPerRectangle& other){
    throw std::runtime_error("append_right not implemented");
  }

  void append_down(const ObjectsPerRectangle& other){
    throw std::runtime_error("append_down not implemented");
  }

  std::vector<Object> objects;
  Rectangle rectangle;
};

struct AllObjects {
  AllObjects() = default;
  AllObjects(const AllObjects &) = default;
  AllObjects(AllObjects &&) = default;
  AllObjects &operator=(const AllObjects &) = default;
  AllObjects &operator=(AllObjects &&) = default;
  AllObjects(size_t rows, size_t cols) : objects_per_rectangle{rows, cols} {}

  ObjectsPerRectangle &get(size_t x, size_t y) {
    return objects_per_rectangle.get(x, y);
  }

  size_t get_rows() const { return objects_per_rectangle.height(); }
private:
  matrix::Matrix<ObjectsPerRectangle> objects_per_rectangle;
};

} // namespace od