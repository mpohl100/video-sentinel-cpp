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

struct ObjectImpl {
  ObjectImpl() = default;
  ObjectImpl(const ObjectImpl &) = default;
  ObjectImpl(ObjectImpl &&) = default;
  ObjectImpl &operator=(const ObjectImpl &) = default;
  ObjectImpl &operator=(ObjectImpl &&) = default;
  ObjectImpl(const Slices &slices) : slices{slices} {}

  std::string to_string() const { return slices.to_string(); }

  bool try_merge_right(ObjectImpl &other) {
    if (slices.touching_right(other.slices)) {
      slices.merge_right(other.slices);
      return true;
    }
    return false;
  }

  bool try_merge_down(ObjectImpl &other, bool debug = false) {
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

  bool touching_right(ObjectImpl &other) {
    return slices.touching_right(other.slices);
  }

  bool touching_down(ObjectImpl &other) {
    return slices.touching_down(other.slices);
  }

  od::Rectangle get_bounding_box() const { return slices.to_rectangle(); }

  bool contains_point(const math2d::Point &point) const {
    return slices.contains_point(point);
  }

  Slices slices;
};

struct Object{
    std::string to_string() const { return object->to_string(); }

  bool try_merge_right(Object other) {
    object->try_merge_right(*other.object);
  }

  bool try_merge_down(Object other, bool debug = false) {
    object->try_merge_down(*other.object, debug);
  }

  bool touching_right(Object other) {
    return object->touching_right(*other.object);
  }

  bool touching_down(Object other) {
    return object->touching_down(*other.object);
  }

  od::Rectangle get_bounding_box() const { return object->get_bounding_box(); }

  bool contains_point(const math2d::Point &point) const {
    return object->contains_point(point);
  }
private:
  std::shared_ptr<od::ObjectImpl> object;
};

} // namespace od