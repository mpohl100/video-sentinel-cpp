#include <catch2/catch_all.hpp>

#include "detection/Object.h"
#include "math2d/math2d.h"

#include <iostream>

namespace {

TEST_CASE("Object", "[object]") {

  SECTION("ObjectTouchingRectangle") {
    const auto slice = od::Slice{math2d::Point{0, 1}, math2d::Point{10, 1}};
  }
}
} // namespace