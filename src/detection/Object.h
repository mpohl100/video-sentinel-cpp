#pragma once

#include "Slices.h"

#include "matrix/Matrix.h"

#include "Rectangle.h"

#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

namespace od {

struct Object {
  Object() = default;
  Object(const Object &) = default;
  Object(Object &&) = default;
  Object &operator=(const Object &) = default;
  Object &operator=(Object &&) = default;
  Object(const Slices &slices) : slices{slices} {}

  bool try_merge_right(Object &other) {
    if (slices.touching_right(other.slices)) {
      slices.merge_right(other.slices);
      return true;
    }
    return false;
  }

  bool try_merge_down(Object &other) {
    if (slices.touching_down(other.slices)) {
      slices.merge_down(other.slices);
      return true;
    }
    return false;
  }

  bool touching_right(Object &other) {
    return slices.touching_right(other.slices);
  }

  bool touching_down(Object &other) {
    return slices.touching_down(other.slices);
  }

  od::Rectangle get_bounding_box() const { return slices.to_rectangle(); }

  Slices slices;
};

} // namespace od