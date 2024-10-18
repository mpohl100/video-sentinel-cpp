#include <catch2/catch_all.hpp>

#include "detection/Object.h"
#include "detection/ObjectsPerRectangle.h"

#include "math2d/math2d.h"

#include <iostream>

namespace {

const auto get_test_slices(const math2d::Point &start,
                           const math2d::Point &end) {
  if (start.y > end.y) {
    throw std::runtime_error("start y must be smaller or equal to end y");
  }
  if (start.x > end.x) {
    throw std::runtime_error("start x must be smaller or equal to end x");
  }
  auto slices = od::Slices{start};
  for (auto y = start.y; y <= end.y; ++y) {
    std::vector<od::AnnotatedSlice> current_line;
    const auto slice =
        od::Slice{math2d::Point{start.x, y}, math2d::Point{end.x, y}};
    current_line.push_back(od::AnnotatedSlice{slice, static_cast<size_t>(y)});
    slices.slices.push_back(current_line);
  }
  return slices;
}

TEST_CASE("Object", "[object]") {

  SECTION("ObjectTouchingRectangle") {
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{10, 1};
    const auto slices = get_test_slices(start, end);
    const auto object = od::Object{slices};

    const auto rectangle_all =
        od::Rectangle{start, math2d::Point{end.x + 1, end.y + 1}};
    CHECK(object.slices.touching_right(rectangle_all));
    CHECK(object.slices.touching_down(rectangle_all));
    CHECK(object.slices.touching_left(rectangle_all));
    CHECK(object.slices.touching_up(rectangle_all));

    const auto rectangle_left =
        od::Rectangle{math2d::Point{start.x, start.y - 1},
                      math2d::Point{end.x + 2, start.y + 2}};
    CHECK(object.slices.touching_left(rectangle_left));
    CHECK_FALSE(object.slices.touching_right(rectangle_left));
    CHECK_FALSE(object.slices.touching_down(rectangle_left));
    CHECK_FALSE(object.slices.touching_up(rectangle_left));

    const auto rectangle_right =
        od::Rectangle{math2d::Point{start.x - 1, start.y - 1},
                      math2d::Point{end.x + 1, start.y + 2}};
    CHECK(object.slices.touching_right(rectangle_right));
    CHECK_FALSE(object.slices.touching_left(rectangle_right));
    CHECK_FALSE(object.slices.touching_down(rectangle_right));
    CHECK_FALSE(object.slices.touching_up(rectangle_right));

    const auto rectangle_up =
        od::Rectangle{math2d::Point{start.x - 1, start.y},
                      math2d::Point{end.x + 2, start.y + 2}};
    CHECK(object.slices.touching_up(rectangle_up));
    CHECK_FALSE(object.slices.touching_left(rectangle_up));
    CHECK_FALSE(object.slices.touching_down(rectangle_up));
    CHECK_FALSE(object.slices.touching_right(rectangle_up));

    const auto rectangle_down =
        od::Rectangle{math2d::Point{start.x - 1, start.y - 1},
                      math2d::Point{end.x + 2, start.y + 1}};
    CHECK(object.slices.touching_down(rectangle_down));
    CHECK_FALSE(object.slices.touching_left(rectangle_down));
    CHECK_FALSE(object.slices.touching_up(rectangle_down));
    CHECK_FALSE(object.slices.touching_right(rectangle_down));
  }
  SECTION("ObjectTouchingRightObject") {
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
  SECTION("ObjectNotTouchingRightObject") {
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
  SECTION("ObjectTouchingDownObject") {
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{10, 1};
    const auto slices = get_test_slices(start, end);
    auto object = od::Object{slices};

    const auto start2 = math2d::Point{0, 2};
    const auto end2 = math2d::Point{10, 2};
    const auto slices2 = get_test_slices(start2, end2);
    auto object2 = od::Object{slices2};

    CHECK(object.try_merge_down(object2));
    // CHECK_FALSE(object.try_merge_down(object2));
    CHECK_FALSE(object.try_merge_right(object2));
  }
  SECTION("ObjectNotTouchingDownObject") {
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
  SECTION("ObjectTouchingDownObjectDiagonally") {
    const auto start = math2d::Point{10, 1};
    const auto end = math2d::Point{20, 1};
    const auto slices = get_test_slices(start, end);
    auto object = od::Object{slices};
    auto second_object = object;

    const auto start2 = math2d::Point{0, 2};
    const auto end2 = math2d::Point{10, 2};
    const auto slices2 = get_test_slices(start2, end2);
    auto object2 = od::Object{slices2};

    CHECK(object.try_merge_down(object2));
    // CHECK_FALSE(object.try_merge_down(object2));
    CHECK_FALSE(object.try_merge_right(object2));

    const auto start3 = math2d::Point{20, 2};
    const auto end3 = math2d::Point{30, 2};
    const auto slices3 = get_test_slices(start3, end3);
    auto object3 = od::Object{slices3};

    CHECK(second_object.try_merge_down(object3));
    // CHECK_FALSE(second_object.try_merge_down(object3));
    CHECK_FALSE(second_object.try_merge_right(object3));
  }
  SECTION("ObjectNotTouchingDownObjectDiagonally") {
    const auto start = math2d::Point{10, 1};
    const auto end = math2d::Point{20, 1};
    const auto slices = get_test_slices(start, end);
    auto object = od::Object{slices};
    auto second_object = object;

    const auto start2 = math2d::Point{0, 2};
    const auto end2 = math2d::Point{8, 2};
    const auto slices2 = get_test_slices(start2, end2);
    auto object2 = od::Object{slices2};

    CHECK_FALSE(object.try_merge_down(object2));
    CHECK_FALSE(object.try_merge_right(object2));

    const auto start3 = math2d::Point{22, 2};
    const auto end3 = math2d::Point{30, 2};
    const auto slices3 = get_test_slices(start3, end3);
    auto object3 = od::Object{slices3};

    CHECK_FALSE(second_object.try_merge_down(object3));
    CHECK_FALSE(second_object.try_merge_right(object3));
  }
  SECTION("ObjectMergingDownHappensMultipleTimes") {
    const auto start = math2d::Point{10, 1};
    const auto end = math2d::Point{40, 10};
    const auto slices = get_test_slices(start, end);
    auto top_object = od::Object{slices};

    const auto start2 = math2d::Point{10, 11};
    const auto end2 = math2d::Point{20, 20};
    const auto slices2 = get_test_slices(start2, end2);
    auto left_object = od::Object{slices2};

    const auto start3 = math2d::Point{21, 11};
    const auto end3 = math2d::Point{30, 20};
    const auto slices3 = get_test_slices(start3, end3);
    auto middle_object = od::Object{slices3};

    const auto start4 = math2d::Point{31, 11};
    const auto end4 = math2d::Point{40, 20};
    const auto slices4 = get_test_slices(start4, end4);
    auto right_object = od::Object{slices4};

    CHECK(top_object.try_merge_down(left_object));
    //std::cout << "After first merge down" << std::endl;
    //std::cout << top_object.to_string() << std::endl;
    CHECK(top_object.try_merge_down(right_object));
    //std::cout << "After second merge down" << std::endl;
    //std::cout << top_object.to_string() << std::endl;
    CHECK(top_object.try_merge_down(middle_object));
    //std::cout << "After third merge down" << std::endl;
    //std::cout << top_object.to_string() << std::endl;
    CHECK_FALSE(top_object.try_merge_down(middle_object));
    //std::cout << "After fourth merge down" << std::endl;
    //std::cout << top_object.to_string() << std::endl;
    CHECK_FALSE(top_object.try_merge_down(left_object));
    //std::cout << "After fifth merge down" << std::endl;
    //std::cout << top_object.to_string() << std::endl;
    CHECK_FALSE(top_object.try_merge_down(right_object));
    //std::cout << "After sixth merge down" << std::endl;
    //std::cout << top_object.to_string() << std::endl;
  }
  SECTION("ObjectsPerRectangleObjectInCorrectTouchingCaches") {
    const auto start = math2d::Point{0, 1};
    const auto end = math2d::Point{10, 3};
    const auto slices = get_test_slices(start, end);
    auto object = std::make_shared<od::Object>(slices);

    // test fitting rectangle touches everything
    auto objects_per_rectangle = od::ObjectsPerRectangle{};
    objects_per_rectangle.set_rectangle(
        od::Rectangle{start, math2d::Point{end.x + 1, end.y + 1}});
    objects_per_rectangle.insert_object(object);

    CHECK(objects_per_rectangle.get_objects().size() == 1);
    CHECK(objects_per_rectangle.get_objects_touching_right().size() == 1);
    CHECK(objects_per_rectangle.get_objects_touching_down().size() == 1);
    CHECK(objects_per_rectangle.get_objects_touching_left().size() == 1);
    CHECK(objects_per_rectangle.get_objects_touching_up().size() == 1);

    // test smaller rectangle touches everything
    auto smaller_objects_per_rectangle = od::ObjectsPerRectangle{};
    smaller_objects_per_rectangle.set_rectangle(
        od::Rectangle{math2d::Point{start.x + 1, start.y + 1},
                      math2d::Point{end.x - 1, end.y - 1}});
    smaller_objects_per_rectangle.insert_object(object);

    CHECK(smaller_objects_per_rectangle.get_objects().size() == 1);
    CHECK(smaller_objects_per_rectangle.get_objects_touching_right().size() ==
          1);
    CHECK(smaller_objects_per_rectangle.get_objects_touching_down().size() ==
          1);
    CHECK(smaller_objects_per_rectangle.get_objects_touching_left().size() ==
          1);
    CHECK(smaller_objects_per_rectangle.get_objects_touching_up().size() == 1);

    // test bigger rectangle touches nothing
    auto bigger_objects_per_rectangle = od::ObjectsPerRectangle{};
    bigger_objects_per_rectangle.set_rectangle(
        od::Rectangle{math2d::Point{start.x - 1, start.y - 1},
                      math2d::Point{end.x + 2, end.y + 2}});
    bigger_objects_per_rectangle.insert_object(object);

    CHECK(bigger_objects_per_rectangle.get_objects().size() == 1);
    CHECK(bigger_objects_per_rectangle.get_objects_touching_right().size() ==
          0);
    CHECK(bigger_objects_per_rectangle.get_objects_touching_down().size() == 0);
    CHECK(bigger_objects_per_rectangle.get_objects_touching_left().size() == 0);
    CHECK(bigger_objects_per_rectangle.get_objects_touching_up().size() == 0);

    // test object touches everything except the right side
    auto no_right_objects_per_rectangle = od::ObjectsPerRectangle{};
    no_right_objects_per_rectangle.set_rectangle(
        od::Rectangle{start, math2d::Point{end.x + 2, end.y + 1}});
    no_right_objects_per_rectangle.insert_object(object);

    CHECK(no_right_objects_per_rectangle.get_objects().size() == 1);
    CHECK(no_right_objects_per_rectangle.get_objects_touching_right().size() ==
          0);
    CHECK(no_right_objects_per_rectangle.get_objects_touching_down().size() ==
          1);
    CHECK(no_right_objects_per_rectangle.get_objects_touching_left().size() ==
          1);
    CHECK(no_right_objects_per_rectangle.get_objects_touching_up().size() == 1);

    // test object touches everything except the left side
    auto no_left_objects_per_rectangle = od::ObjectsPerRectangle{};
    no_left_objects_per_rectangle.set_rectangle(
        od::Rectangle{math2d::Point{start.x - 1, start.y},
                      math2d::Point{end.x + 1, end.y + 1}});
    no_left_objects_per_rectangle.insert_object(object);

    CHECK(no_left_objects_per_rectangle.get_objects().size() == 1);
    CHECK(no_left_objects_per_rectangle.get_objects_touching_right().size() ==
          1);
    CHECK(no_left_objects_per_rectangle.get_objects_touching_down().size() ==
          1);
    CHECK(no_left_objects_per_rectangle.get_objects_touching_left().size() ==
          0);
    CHECK(no_left_objects_per_rectangle.get_objects_touching_up().size() == 1);

    // test object touches everything except the up side
    auto no_up_objects_per_rectangle = od::ObjectsPerRectangle{};
    no_up_objects_per_rectangle.set_rectangle(
        od::Rectangle{math2d::Point{start.x, start.y - 1},
                      math2d::Point{end.x + 1, end.y + 1}});
    no_up_objects_per_rectangle.insert_object(object);

    CHECK(no_up_objects_per_rectangle.get_objects().size() == 1);
    CHECK(no_up_objects_per_rectangle.get_objects_touching_right().size() == 1);
    CHECK(no_up_objects_per_rectangle.get_objects_touching_down().size() == 1);
    CHECK(no_up_objects_per_rectangle.get_objects_touching_left().size() == 1);
    CHECK(no_up_objects_per_rectangle.get_objects_touching_up().size() == 0);

    // test object touches everything except the down side
    auto no_down_objects_per_rectangle = od::ObjectsPerRectangle{};
    no_down_objects_per_rectangle.set_rectangle(od::Rectangle{
        math2d::Point{start.x, start.y}, math2d::Point{end.x + 1, end.y + 2}});
    no_down_objects_per_rectangle.insert_object(object);

    CHECK(no_down_objects_per_rectangle.get_objects().size() == 1);
    CHECK(no_down_objects_per_rectangle.get_objects_touching_right().size() ==
          1);
    CHECK(no_down_objects_per_rectangle.get_objects_touching_down().size() ==
          0);
    CHECK(no_down_objects_per_rectangle.get_objects_touching_left().size() ==
          1);
    CHECK(no_down_objects_per_rectangle.get_objects_touching_up().size() == 1);
  }
  SECTION("ObjectsPerRectangleMergingRight") {
    const auto rectangle_left =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 10}};
    auto objects_per_rectangle_left = od::ObjectsPerRectangle{};
    objects_per_rectangle_left.set_rectangle(rectangle_left);

    auto inside_object_left = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{2, 2}));
    auto touching_object_right = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 1}, math2d::Point{9, 4}));
    objects_per_rectangle_left.insert_object(inside_object_left);
    objects_per_rectangle_left.insert_object(touching_object_right);

    CHECK(objects_per_rectangle_left.get_objects().size() == 2);
    CHECK(objects_per_rectangle_left.get_objects_touching_right().size() == 1);
    CHECK(objects_per_rectangle_left.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_up().size() == 0);

    const auto rectangle_right =
        od::Rectangle{math2d::Point{10, 0}, math2d::Point{20, 10}};
    auto objects_per_rectangle_right = od::ObjectsPerRectangle{};
    objects_per_rectangle_right.set_rectangle(rectangle_right);

    auto inside_object_right = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{15, 1}, math2d::Point{16, 2}));
    auto touching_object_left = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{10, 1}, math2d::Point{14, 4}));
    objects_per_rectangle_right.insert_object(inside_object_right);
    objects_per_rectangle_right.insert_object(touching_object_left);

    CHECK(objects_per_rectangle_right.get_objects().size() == 2);
    CHECK(objects_per_rectangle_right.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_right.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_right.get_objects_touching_left().size() == 1);
    CHECK(objects_per_rectangle_right.get_objects_touching_up().size() == 0);

    objects_per_rectangle_left.append_right(objects_per_rectangle_right);

    CHECK(objects_per_rectangle_left.get_rectangle().to_math2d_rectangle() ==
          od::Rectangle{math2d::Point{0, 0}, math2d::Point{20, 10}}
              .to_math2d_rectangle());
    CHECK(objects_per_rectangle_left.get_objects().size() == 3);
    CHECK(objects_per_rectangle_left.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_up().size() == 0);
  }
  SECTION("ObjectsPerRectangleMergingRightNoObjectMergeCandidateRight") {
    const auto rectangle_left =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 10}};
    auto objects_per_rectangle_left = od::ObjectsPerRectangle{};
    objects_per_rectangle_left.set_rectangle(rectangle_left);

    auto inside_object_left = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{2, 2}));
    auto touching_object_right = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 1}, math2d::Point{9, 4}));
    objects_per_rectangle_left.insert_object(inside_object_left);
    objects_per_rectangle_left.insert_object(touching_object_right);

    CHECK(objects_per_rectangle_left.get_objects().size() == 2);
    CHECK(objects_per_rectangle_left.get_objects_touching_right().size() == 1);
    CHECK(objects_per_rectangle_left.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_up().size() == 0);

    const auto rectangle_right =
        od::Rectangle{math2d::Point{10, 0}, math2d::Point{20, 10}};
    auto objects_per_rectangle_right = od::ObjectsPerRectangle{};
    objects_per_rectangle_right.set_rectangle(rectangle_right);

    auto inside_object_right = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{15, 1}, math2d::Point{16, 2}));
    auto touching_object_left_candidate = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{11, 1}, math2d::Point{14, 4}));
    objects_per_rectangle_right.insert_object(inside_object_right);
    objects_per_rectangle_right.insert_object(touching_object_left_candidate);

    CHECK(objects_per_rectangle_right.get_objects().size() == 2);
    CHECK(objects_per_rectangle_right.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_right.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_right.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_right.get_objects_touching_up().size() == 0);

    objects_per_rectangle_left.append_right(objects_per_rectangle_right);

    CHECK(objects_per_rectangle_left.get_rectangle().to_math2d_rectangle() ==
          od::Rectangle{math2d::Point{0, 0}, math2d::Point{20, 10}}
              .to_math2d_rectangle());
    CHECK(objects_per_rectangle_left.get_objects().size() == 4);
    CHECK(objects_per_rectangle_left.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_up().size() == 0);
  }
  SECTION("ObjectsPerRectangleMergingRightNoObjectMergeCandidateLeft") {
    const auto rectangle_left =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 10}};
    auto objects_per_rectangle_left = od::ObjectsPerRectangle{};
    objects_per_rectangle_left.set_rectangle(rectangle_left);

    auto inside_object_left = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{2, 2}));
    auto touching_object_right_candidate = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 1}, math2d::Point{8, 4}));
    objects_per_rectangle_left.insert_object(inside_object_left);
    objects_per_rectangle_left.insert_object(touching_object_right_candidate);

    CHECK(objects_per_rectangle_left.get_objects().size() == 2);
    CHECK(objects_per_rectangle_left.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_up().size() == 0);

    const auto rectangle_right =
        od::Rectangle{math2d::Point{10, 0}, math2d::Point{20, 10}};
    auto objects_per_rectangle_right = od::ObjectsPerRectangle{};
    objects_per_rectangle_right.set_rectangle(rectangle_right);

    auto inside_object_right = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{15, 1}, math2d::Point{16, 2}));
    auto touching_object_left = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{10, 1}, math2d::Point{14, 4}));
    objects_per_rectangle_right.insert_object(inside_object_right);
    objects_per_rectangle_right.insert_object(touching_object_left);

    CHECK(objects_per_rectangle_right.get_objects().size() == 2);
    CHECK(objects_per_rectangle_right.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_right.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_right.get_objects_touching_left().size() == 1);
    CHECK(objects_per_rectangle_right.get_objects_touching_up().size() == 0);

    objects_per_rectangle_left.append_right(objects_per_rectangle_right);

    CHECK(objects_per_rectangle_left.get_rectangle().to_math2d_rectangle() ==
          od::Rectangle{math2d::Point{0, 0}, math2d::Point{20, 10}}
              .to_math2d_rectangle());
    CHECK(objects_per_rectangle_left.get_objects().size() == 4);
    CHECK(objects_per_rectangle_left.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_left.get_objects_touching_up().size() == 0);
  }
  SECTION("ObjectsPerRectangleAppendDown") {
    const auto rectangle_up =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 10}};
    auto objects_per_rectangle_up = od::ObjectsPerRectangle{};
    objects_per_rectangle_up.set_rectangle(rectangle_up);

    auto inside_object = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{2, 2}));
    auto touching_object_down = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 4}, math2d::Point{8, 9}));
    objects_per_rectangle_up.insert_object(inside_object);
    objects_per_rectangle_up.insert_object(touching_object_down);

    CHECK(objects_per_rectangle_up.get_objects().size() == 2);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 1);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);

    const auto rectangle_down =
        od::Rectangle{math2d::Point{0, 10}, math2d::Point{10, 20}};
    auto objects_per_rectangle_down = od::ObjectsPerRectangle{};
    objects_per_rectangle_down.set_rectangle(rectangle_down);

    auto inside_object_down = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 15}, math2d::Point{2, 16}));
    auto touching_object_up = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 10}, math2d::Point{8, 14}));
    objects_per_rectangle_down.insert_object(inside_object_down);
    objects_per_rectangle_down.insert_object(touching_object_up);

    CHECK(objects_per_rectangle_down.get_objects().size() == 2);
    CHECK(objects_per_rectangle_down.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_up().size() == 1);

    objects_per_rectangle_up.append_down(objects_per_rectangle_down);

    CHECK(objects_per_rectangle_up.get_rectangle().to_math2d_rectangle() ==
          od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 20}}
              .to_math2d_rectangle());
    CHECK(objects_per_rectangle_up.get_objects().size() == 3);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);
  }
  SECTION("ObjectsPerRectangleAppendDownNoMergingCauseOfUp") {
    const auto rectangle_up =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 10}};
    auto objects_per_rectangle_up = od::ObjectsPerRectangle{};
    objects_per_rectangle_up.set_rectangle(rectangle_up);

    auto inside_object = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{2, 2}));
    auto touching_object_down_candidate = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 4}, math2d::Point{8, 8}));
    objects_per_rectangle_up.insert_object(inside_object);
    objects_per_rectangle_up.insert_object(touching_object_down_candidate);

    CHECK(objects_per_rectangle_up.get_objects().size() == 2);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);

    const auto rectangle_down =
        od::Rectangle{math2d::Point{0, 10}, math2d::Point{10, 20}};
    auto objects_per_rectangle_down = od::ObjectsPerRectangle{};
    objects_per_rectangle_down.set_rectangle(rectangle_down);

    auto inside_object_down = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 15}, math2d::Point{2, 16}));
    auto touching_object_up = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 10}, math2d::Point{8, 14}));
    objects_per_rectangle_down.insert_object(inside_object_down);
    objects_per_rectangle_down.insert_object(touching_object_up);

    CHECK(objects_per_rectangle_down.get_objects().size() == 2);
    CHECK(objects_per_rectangle_down.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_up().size() == 1);

    objects_per_rectangle_up.append_down(objects_per_rectangle_down);

    CHECK(objects_per_rectangle_up.get_rectangle().to_math2d_rectangle() ==
          od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 20}}
              .to_math2d_rectangle());
    CHECK(objects_per_rectangle_up.get_objects().size() == 4);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);
  }
  SECTION("ObjectsPerRectangleAppendDownNoMergingCauseOfDown") {
    const auto rectangle_up =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 10}};
    auto objects_per_rectangle_up = od::ObjectsPerRectangle{};
    objects_per_rectangle_up.set_rectangle(rectangle_up);

    auto inside_object = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{2, 2}));
    auto touching_object_down = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 4}, math2d::Point{8, 9}));
    objects_per_rectangle_up.insert_object(inside_object);
    objects_per_rectangle_up.insert_object(touching_object_down);

    CHECK(objects_per_rectangle_up.get_objects().size() == 2);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 1);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);

    const auto rectangle_down =
        od::Rectangle{math2d::Point{0, 10}, math2d::Point{10, 20}};
    auto objects_per_rectangle_down = od::ObjectsPerRectangle{};
    objects_per_rectangle_down.set_rectangle(rectangle_down);

    auto inside_object_down = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 15}, math2d::Point{2, 16}));
    auto touching_object_up_candidate = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{4, 11}, math2d::Point{8, 14}));
    objects_per_rectangle_down.insert_object(inside_object_down);
    objects_per_rectangle_down.insert_object(touching_object_up_candidate);

    CHECK(objects_per_rectangle_down.get_objects().size() == 2);
    CHECK(objects_per_rectangle_down.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_up().size() == 0);

    objects_per_rectangle_up.append_down(objects_per_rectangle_down);

    CHECK(objects_per_rectangle_up.get_rectangle().to_math2d_rectangle() ==
          od::Rectangle{math2d::Point{0, 0}, math2d::Point{10, 20}}
              .to_math2d_rectangle());
    CHECK(objects_per_rectangle_up.get_objects().size() == 4);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);
  }
  SECTION("ObjectsPerRectangleMergeDownMergesTwoAdjacentObjectsToTheBottom") {
    const auto rectangle_up =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{100, 100}};
    auto objects_per_rectangle_up = od::ObjectsPerRectangle{};
    objects_per_rectangle_up.set_rectangle(rectangle_up);

    auto touching_object_down = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{98, 99}));
    objects_per_rectangle_up.insert_object(touching_object_down);
    CHECK(objects_per_rectangle_up.get_objects().size() == 1);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 1);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);

    const auto rectangle_down =
        od::Rectangle{math2d::Point{0, 100}, math2d::Point{100, 200}};
    auto objects_per_rectangle_down = od::ObjectsPerRectangle{};
    objects_per_rectangle_down.set_rectangle(rectangle_down);

    auto touching_object_up = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 100}, math2d::Point{49, 150}));
    objects_per_rectangle_down.insert_object(touching_object_up);

    auto touching_object_up_2 = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{51, 100}, math2d::Point{98, 150}));
    objects_per_rectangle_down.insert_object(touching_object_up_2);

    CHECK(objects_per_rectangle_down.get_objects().size() == 2);
    CHECK(objects_per_rectangle_down.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_down.get_objects_touching_up().size() == 2);

    objects_per_rectangle_up.append_down(objects_per_rectangle_down);
    CHECK(objects_per_rectangle_up.get_objects().size() == 1);
    CHECK(objects_per_rectangle_up.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_down().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle_up.get_objects_touching_up().size() == 0);
  }

  SECTION("ObjectMergesDownTwice") {
    const auto rectangle =
        od::Rectangle{math2d::Point{0, 0}, math2d::Point{100, 100}};
    auto objects_per_rectangle = od::ObjectsPerRectangle{};
    objects_per_rectangle.set_rectangle(rectangle);

    auto touching_object_down = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 1}, math2d::Point{98, 99}));
    objects_per_rectangle.insert_object(touching_object_down);

    CHECK(objects_per_rectangle.get_objects().size() == 1);
    CHECK(objects_per_rectangle.get_objects_touching_right().size() == 0);
    CHECK(objects_per_rectangle.get_objects_touching_down().size() == 1);
    CHECK(objects_per_rectangle.get_objects_touching_left().size() == 0);
    CHECK(objects_per_rectangle.get_objects_touching_up().size() == 0);

    const auto down_rectangle =
        od::Rectangle{math2d::Point{0, 100}, math2d::Point{100, 200}};
    auto down_objects_per_rectangle = od::ObjectsPerRectangle{};
    down_objects_per_rectangle.set_rectangle(down_rectangle);

    const auto touching_object_up_1 = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{1, 100}, math2d::Point{49, 150}));
    down_objects_per_rectangle.insert_object(touching_object_up_1);
    const auto touching_object_up_2 = std::make_shared<od::Object>(
        get_test_slices(math2d::Point{51, 100}, math2d::Point{98, 150}));
    down_objects_per_rectangle.insert_object(touching_object_up_2);

    CHECK(down_objects_per_rectangle.get_objects().size() == 2);
    CHECK(down_objects_per_rectangle.get_objects_touching_right().size() == 0);
    CHECK(down_objects_per_rectangle.get_objects_touching_down().size() == 0);
    CHECK(down_objects_per_rectangle.get_objects_touching_left().size() == 0);
    CHECK(down_objects_per_rectangle.get_objects_touching_up().size() == 2);

    objects_per_rectangle.append_down(down_objects_per_rectangle);

    CHECK(objects_per_rectangle.get_objects().size() == 1);
  }
}

} // namespace