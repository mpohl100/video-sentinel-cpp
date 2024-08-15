#pragma once

#include "ObjectsPerRectangle.h"
#include "matrix/Matrix.h"

namespace od {

struct AllObjects {
  AllObjects() = default;
  AllObjects(const AllObjects &) = default;
  AllObjects(AllObjects &&) = default;
  AllObjects &operator=(const AllObjects &) = default;
  AllObjects &operator=(AllObjects &&) = default;
  AllObjects(size_t rows, size_t cols) : objects_per_rectangle{cols, rows} {}

  ObjectsPerRectangle &get(size_t row, size_t col) {
    return objects_per_rectangle.get(col, row);
  }

  size_t get_rows() const { return objects_per_rectangle.height(); }
  size_t get_cols() const { return objects_per_rectangle.width(); }

private:
  matrix::Matrix<ObjectsPerRectangle> objects_per_rectangle;
};

} // namespace od