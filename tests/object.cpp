#include <catch2/catch_all.hpp>

#include "detection/Object.h"
#include "math2d/math2d.h"

#include <iostream>

namespace {

const auto get_test_slices(const math2d::Point &start, const math2d::Point &end) {
  if(start.y != end.y) {
    throw std::runtime_error("start and end y must be the same");
  }
  const auto slice = od::Slice{start, end};
  auto slices = od::Slices{slice.start};
  std::vector<od::AnnotatedSlice> current_line;
  current_line.push_back(od::AnnotatedSlice{slice, static_cast<size_t>(start.y)});
  slices.slices.push_back(current_line);
  return slices;
}

TEST_CASE("Object", "[object]") {

  SECTION("ObjectTouchingRectangle") {
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{10, 1};
    const auto slices = get_test_slices(start, end);
    const auto object = od::Object{slices};

    const auto rectangle_all = od::Rectangle{start, math2d::Point{end.x + 1, end.y + 1}};
    CHECK(object.slices.touching_right(rectangle_all));
    CHECK(object.slices.touching_down(rectangle_all));
    CHECK(object.slices.touching_left(rectangle_all));
    CHECK(object.slices.touching_up(rectangle_all));

    const auto rectangle_left = od::Rectangle{math2d::Point{start.x, start.y - 1}, math2d::Point{end.x + 2, start.y + 2}};
    CHECK(object.slices.touching_left(rectangle_left));
    CHECK_FALSE(object.slices.touching_right(rectangle_left));
    CHECK_FALSE(object.slices.touching_down(rectangle_left));
    CHECK_FALSE(object.slices.touching_up(rectangle_left));

    const auto rectangle_right = od::Rectangle{math2d::Point{start.x - 1, start.y - 1}, math2d::Point{end.x + 1, start.y + 2}};
    CHECK(object.slices.touching_right(rectangle_right));
    CHECK_FALSE(object.slices.touching_left(rectangle_right));
    CHECK_FALSE(object.slices.touching_down(rectangle_right));
    CHECK_FALSE(object.slices.touching_up(rectangle_right));

    const auto rectangle_up = od::Rectangle{math2d::Point{start.x - 1, start.y}, math2d::Point{end.x + 2, start.y + 2}};
    CHECK(object.slices.touching_up(rectangle_up));
    CHECK_FALSE(object.slices.touching_left(rectangle_up));
    CHECK_FALSE(object.slices.touching_down(rectangle_up));
    CHECK_FALSE(object.slices.touching_right(rectangle_up));

    const auto rectangle_down = od::Rectangle{math2d::Point{start.x - 1, start.y - 1}, math2d::Point{end.x + 2, start.y + 1}};
    CHECK(object.slices.touching_down(rectangle_down));
    CHECK_FALSE(object.slices.touching_left(rectangle_down));
    CHECK_FALSE(object.slices.touching_up(rectangle_down));
    CHECK_FALSE(object.slices.touching_right(rectangle_down));
  }
  SECTION("ObjectTouchingRightObject"){
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{10, 1};
    const auto slices = get_test_slices(start, end);
    auto object = od::Object{slices};

    const auto start2 = math2d::Point{11, 1};
    const auto end2 = math2d::Point{20, 1};
    const auto slices2 = get_test_slices(start2, end2);
    auto object2 = od::Object{slices2};

    CHECK(object.try_merge_right(object2));
    CHECK_FALSE(object.try_merge_right(object2));
    CHECK_FALSE(object.try_merge_down(object2));
  }
  SECTION("ObjectNotTouchingRightObject"){
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{9, 1};
    const auto slices = get_test_slices(start, end);
    auto object = od::Object{slices};

    const auto start2 = math2d::Point{11, 1};
    const auto end2 = math2d::Point{20, 1};
    const auto slices2 = get_test_slices(start2, end2);
    auto object2 = od::Object{slices2};

    CHECK_FALSE(object.try_merge_right(object2));
    CHECK_FALSE(object.try_merge_down(object2));
  }
  SECTION("ObjectTouchingDownObject"){
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{10, 1};
    const auto slices = get_test_slices(start, end);
    auto object = od::Object{slices};

    const auto start2 = math2d::Point{0, 2};
    const auto end2 = math2d::Point{10, 2};
    const auto slices2 = get_test_slices(start2, end2);
    auto object2 = od::Object{slices2};

    CHECK(object.try_merge_down(object2));
    CHECK_FALSE(object.try_merge_down(object2));
    CHECK_FALSE(object.try_merge_right(object2));
  }
  SECTION("ObjectNotTouchingDownObject"){
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{10, 1};
    const auto slices = get_test_slices(start, end);
    auto object = od::Object{slices};

    const auto start2 = math2d::Point{0, 3};
    const auto end2 = math2d::Point{10, 3};
    const auto slices2 = get_test_slices(start2, end2);
    auto object2 = od::Object{slices2};

    CHECK_FALSE(object.try_merge_down(object2));
    CHECK_FALSE(object.try_merge_right(object2));
  }
}

} // namespace