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
    bool do_log = false;
    if (get_bounding_box().to_math2d_rectangle().area() > 100 &&
        other.get_bounding_box().to_math2d_rectangle().area() > 100) {
      do_log = true;
      std::cout << "Encountered two bigger rectangles";
    }
    if (slices.touching_right(other.slices)) {
      slices.merge_right(other.slices);
      if (do_log) {
        std::cout << "Merged right: " << slices.to_rectangle().to_string()
                  << " with " << other.slices.to_rectangle().to_string()
                  << std::endl;
      }
      return true;
    }
    if (do_log) {
      std::cout << "Did not merge right: " << slices.to_rectangle().to_string()
                << " with " << other.slices.to_rectangle().to_string()
                << std::endl;
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