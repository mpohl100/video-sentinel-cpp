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

  std::string to_string() const { return slices.to_string(); }

  bool try_merge_right(Object &other) {
    if (slices.touching_right(other.slices)) {
      slices.merge_right(other.slices);
      return true;
    }
    return false;
  }

  bool try_merge_down(Object &other, bool debug = false) {
    if (debug) {
      std::cout << "try_merge_down" << std::endl;
      std::cout << "this: " << slices.to_string() << std::endl;
      std::cout << "other: " << other.slices.to_string() << std::endl;
    }
    if (slices.touching_down(other.slices)) {
      slices.merge_down(other.slices, debug);
      if (debug) {
        std::cout << "merged: " << slices.to_string() << std::endl;
      }
      return true;
    }
    if (debug) {
      std::cout << "not merged" << std::endl;
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

  bool contains_point(const math2d::Point &point) const {
    return slices.contains_point(point);
  }

  Slices slices;
};

} // namespace od