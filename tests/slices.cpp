#include <catch2/catch_all.hpp>

#include "webcam/webcam.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

namespace {

TEST_CASE("Slices", "[slices]") {

  SECTION("SlicesCreatesSnakeAsOneObject") {
    // Arrange

    auto slices = od::Slices{math2d::Point{0, 0}};
    // construct first line
    auto slice = od::AnnotatedSlice{od::Slice{math2d::Point{0, 0}, math2d::Point{10, 0}}, 0};
    auto neighbour_slice = od::AnnotatedSlice{od::Slice{math2d::Point{21, 0}, math2d::Point{50, 0}}, 0};
    auto slice_line = od::SliceLine{std::vector<od::AnnotatedSlice>{slice, neighbour_slice}, 0};
    slices.slices.push_back(slice_line);
    // construct second line
    auto second_slice = od::AnnotatedSlice{od::Slice{math2d::Point{0, 1}, math2d::Point{10, 1}}, 1};
    auto second_neighbour_slice = od::AnnotatedSlice{od::Slice{math2d::Point{21, 1}, math2d::Point{30, 1}}, 1};
    auto second_neighbour_slice2 = od::AnnotatedSlice{od::Slice{math2d::Point{41, 1}, math2d::Point{50, 1}}, 1};
    auto second_slice_line = od::SliceLine{std::vector<od::AnnotatedSlice>{second_slice, second_neighbour_slice, second_neighbour_slice2}, 1};
    slices.slices.push_back(second_slice_line);
    // construct third line
    auto third_slice = od::AnnotatedSlice{od::Slice{math2d::Point{0, 2}, math2d::Point{10, 2}}, 2};
    auto third_neighbour_slice = od::AnnotatedSlice{od::Slice{math2d::Point{21, 2}, math2d::Point{30, 2}}, 2};
    auto third_neighbour_slice2 = od::AnnotatedSlice{od::Slice{math2d::Point{41, 2}, math2d::Point{50, 2}}, 2};
    auto third_slice_line = od::SliceLine{std::vector<od::AnnotatedSlice>{third_slice, third_neighbour_slice, third_neighbour_slice2}, 2};
    slices.slices.push_back(third_slice_line);
    // construct fourth line
    auto fourth_slice = od::AnnotatedSlice{od::Slice{math2d::Point{0, 3}, math2d::Point{30, 3}}, 3};
    auto fourth_neighbour_slice = od::AnnotatedSlice{od::Slice{math2d::Point{41, 3}, math2d::Point{50, 3}}, 3};
    auto fourth_slice_line = od::SliceLine{std::vector<od::AnnotatedSlice>{fourth_slice, fourth_neighbour_slice}, 3};
    slices.slices.push_back(fourth_slice_line);

    // Act
    auto objects = od::deduce_objects(slices);

    // Assert
    CHECK(objects.size() == 1);
  }

}

}  // namespace