#include <catch2/catch_all.hpp>

#include "deduction/ObjectTrace.h"
#include "math2d/math2d.h"
#include "webcam/webcam.h"

#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

cv::Mat create_image_with_square(int degrees) {
  // generate an all white image with a black square in the middle
  cv::Mat img(30, 30, CV_8UC3, cv::Scalar(0, 0, 0));
  cv::rectangle(img, cv::Point(8, 8), cv::Point(22, 22),
                cv::Scalar(255, 255, 255), -1);
  // rotate the rectangle by degrees
  cv::Mat M = cv::getRotationMatrix2D(cv::Point(15, 15), degrees, 1);
  cv::warpAffine(img, img, M, img.size());
  return img;
}

od::Object deduce_object(par::Executor &executor, const cv::Mat &img) {
  auto frame_data = webcam::FrameData{img};
  auto flow = webcam::process_frame_single_loop(frame_data, img);
  executor.run(flow);
  executor.wait_for(flow);
  auto objects = frame_data.result_objects.get_objects();
  CHECK(objects.size() == 2);
  std::sort(objects.begin(), objects.end(),
            [](const auto &lhs, const auto &rhs) {
              return lhs.get_bounding_box().to_math2d_rectangle().area() <
                     rhs.get_bounding_box().to_math2d_rectangle().area();
            });
  return objects[0];
}

std::optional<od::Object>
deduce_object_at_position(par::Executor &executor, const cv::Mat &img,
                          const math2d::CoordinatedPoint &position) {
  auto frame_data = webcam::FrameData{img};
  auto flow = webcam::process_frame_single_loop(frame_data, img);
  executor.run(flow);
  executor.wait_for(flow);
  auto objects = frame_data.result_objects.get_objects();
  for (const auto object : objects) {
    if (object.contains_point(position)) {
      return object;
    }
  }
  return std::nullopt;
}

std::vector<od::Object> deduce_all_objects(par::Executor &executor,
                                           const cv::Mat &img) {
  auto frame_data = webcam::FrameData{img};
  auto flow = webcam::process_frame_single_loop(frame_data, img);
  executor.run(flow);
  executor.wait_for(flow);
  auto objects = frame_data.result_objects.get_objects();
  std::sort(objects.begin(), objects.end(),
            [](const auto &lhs, const auto &rhs) {
              return lhs.get_bounding_box().to_math2d_rectangle().area() <
                     rhs.get_bounding_box().to_math2d_rectangle().area();
            });
  return objects;
}

template <typename T> struct ColoredObject {
  ColoredObject(T object, std::string color, int rotation_angle_degrees)
      : object{object}, color{color}, rotation_angle_degrees{
                                          rotation_angle_degrees} {}
  ColoredObject() = default;
  ColoredObject(const ColoredObject &) = default;
  ColoredObject(ColoredObject &&) = default;
  ColoredObject &operator=(const ColoredObject &) = default;
  ColoredObject &operator=(ColoredObject &&) = default;

  T object;
  std::string color;
  int rotation_angle_degrees = 0;
};

struct ObjectsData {
  std::vector<ColoredObject<math2d::Rectangle>> rectangles;
  std::vector<ColoredObject<math2d::Circle>> circles;
};

// Include center point of your rectangle, size of your rectangle and the
// degrees of rotation
void draw_rotated_rectangle(cv::Mat &image, const math2d::Rectangle &rectangle,
                            cv::Scalar color, double rotationDegrees) {
  const auto center = rectangle.center();
  cv::Point centerPoint =
      cv::Point{static_cast<int>(center.x), static_cast<int>(center.y)};
  cv::Size rectangleSize = cv::Size{static_cast<int>(rectangle.width()),
                                    static_cast<int>(rectangle.height())};
  // Create the rotated rectangle
  cv::RotatedRect rotatedRectangle(centerPoint, rectangleSize, rotationDegrees);

  // We take the edges that OpenCV calculated for us
  cv::Point2f vertices2f[4];
  rotatedRectangle.points(vertices2f);

  // Convert them so we can use them in a fillConvexPoly
  cv::Point vertices[4];
  for (int i = 0; i < 4; ++i) {
    vertices[i] = vertices2f[i];
  }

  // Now we can fill the rotated rectangle with our specified color
  cv::fillConvexPoly(image, vertices, 4, color);
}

cv::Scalar to_scalar(std::string color) {
  if (color == "red") {
    return cv::Scalar(0, 0, 255);
  } else if (color == "green") {
    return cv::Scalar(0, 255, 0);
  } else if (color == "blue") {
    return cv::Scalar(255, 0, 0);
  } else {
    return cv::Scalar(255, 255, 255);
  }
}

cv::Mat create_test_image_with_objects(const ObjectsData &objects_data,
                                       int dimX, int dimY) {
  cv::Mat img(dimX, dimY, CV_8UC3, cv::Scalar(0, 0, 0));
  for (const auto &rectangle : objects_data.rectangles) {
    draw_rotated_rectangle(img, rectangle.object, to_scalar(rectangle.color),
                           rectangle.rotation_angle_degrees);
  }
  for (const auto &circle : objects_data.circles) {
    cv::circle(img,
               cv::Point(static_cast<int>(circle.object.center().x),
                         static_cast<int>(circle.object.center().y)),
               circle.object.radius(), to_scalar(circle.color), -1);
  }
  return img;
}

ObjectsData generate_object_data() {
  // generate a four squares
  const auto target_square_1 =
      math2d::Rectangle{math2d::Point{5, 5}, math2d::Point{15, 15}};
  const auto target_square_2 =
      math2d::Rectangle{math2d::Point{25, 5}, math2d::Point{35, 15}};
  const auto target_square_3 =
      math2d::Rectangle{math2d::Point{45, 5}, math2d::Point{65, 25}};
  const auto target_square_4 =
      math2d::Rectangle{math2d::Point{85, 5}, math2d::Point{115, 35}};

  // generate five circles
  const auto target_circle_1 = math2d::Circle{math2d::Point{5, 55}, 5};
  const auto target_circle_2 = math2d::Circle{math2d::Point{25, 55}, 5};
  const auto target_circle_3 = math2d::Circle{math2d::Point{45, 55}, 10};
  const auto target_circle_4 = math2d::Circle{math2d::Point{85, 55}, 15};
  const auto target_circle_5 = math2d::Circle{math2d::Point{115, 55}, 20};

  // generate three rectangles of the same form
  const auto target_rect_1 =
      math2d::Rectangle{math2d::Point{5, 85}, math2d::Point{15, 105}};
  const auto target_rect_2 =
      math2d::Rectangle{math2d::Point{25, 85}, math2d::Point{35, 105}};
  const auto target_rect_3 =
      math2d::Rectangle{math2d::Point{55, 85}, math2d::Point{75, 125}};

  auto objects_data = ObjectsData{};
  objects_data.rectangles.push_back(
      ColoredObject{target_square_1, std::string("red"), 0});
  objects_data.rectangles.push_back(
      ColoredObject{target_square_2, std::string("green"), 30});
  objects_data.rectangles.push_back(
      ColoredObject{target_square_3, std::string("blue"), 60});
  objects_data.rectangles.push_back(
      ColoredObject{target_square_4, std::string("white"), 90});

  objects_data.circles.push_back(
      ColoredObject{target_circle_1, std::string("red"), 0});
  objects_data.circles.push_back(
      ColoredObject{target_circle_2, std::string("green"), 30});
  objects_data.circles.push_back(
      ColoredObject{target_circle_3, std::string("blue"), 60});
  objects_data.circles.push_back(
      ColoredObject{target_circle_4, std::string("white"), 90});
  objects_data.circles.push_back(
      ColoredObject{target_circle_5, std::string("black"), 120});

  objects_data.rectangles.push_back(
      ColoredObject{target_rect_1, std::string("red"), 0});
  objects_data.rectangles.push_back(
      ColoredObject{target_rect_2, std::string("green"), 30});
  objects_data.rectangles.push_back(
      ColoredObject{target_rect_3, std::string("blue"), 60});

  return objects_data;
}

TEST_CASE("Trace", "[trace]") {

  SECTION("TraceDetectsSquareInMultipleDirections") {
    auto executor = par::Executor(1);
    auto img0 = create_image_with_square(0);
    auto obj0 = deduce_object(executor, img0);
    CHECK(obj0.get_bounding_box().to_math2d_rectangle().area() == 100);
    const auto skeleton_params = deduct::SkeletonParams{30, 30};
    auto object0_trace = deduct::ObjectTrace{obj0, skeleton_params}.get_trace();
    CHECK(object0_trace.get_ratios().size() == 6);
  }

  SECTION("TwoSameObjectsOfDifferentImagesHaveTheSameForm") {
    auto executor = par::Executor(1);
    auto img0 = create_image_with_square(0);
    auto img1 = create_image_with_square(0);
    auto obj0 = deduce_object(executor, img0);
    auto obj1 = deduce_object(executor, img1);
    CHECK(obj0.get_bounding_box().to_math2d_rectangle().area() == 100);
    CHECK(obj1.get_bounding_box().to_math2d_rectangle().area() == 100);
    const auto skeleton_params = deduct::SkeletonParams{30, 30};
    auto object0_trace = deduct::ObjectTrace{obj0, skeleton_params}.get_trace();
    auto object1_trace = deduct::ObjectTrace{obj1, skeleton_params}.get_trace();

    std::cout << "Object trace 0: " << object0_trace.to_string() << std::endl;

    const auto same_outer = object0_trace.compare(
        object1_trace, deduct::ComparisonParams{0.1, true, true, 0.9});
    CHECK(same_outer);

    const auto same_inner = object0_trace.compare(
        object1_trace, deduct::ComparisonParams{0.1, false, true, 0.9});
    CHECK(same_inner);

    const auto same_outer_integral = object0_trace.compare(
        object1_trace, deduct::ComparisonParams{0.1, true, false, 0.9});
    CHECK(same_outer);

    const auto same_inner_integral = object0_trace.compare(
        object1_trace, deduct::ComparisonParams{0.1, false, false, 0.9});
    CHECK(same_inner);
  }

  SECTION("TraceDetectsTheCorrectNumberOfSimilarSquares") {
    // Arrange
    auto executor = par::Executor(1);
    const auto reference_square =
        math2d::Rectangle{math2d::Point{15, 15}, math2d::Point{25, 25}};
    auto reference_objects = ObjectsData{};
    reference_objects.rectangles.push_back(
        ColoredObject{reference_square, std::string("red"), 0});
    const auto reference_image =
        create_test_image_with_objects(reference_objects, 50, 50);
    const auto coordinate_system = math2d::CoordinateSystem{
        math2d::Point{0, 0}, math2d::Vector{1, 0}, math2d::Vector{0, 1}};
    const auto reference_object = deduce_object_at_position(
        executor, reference_image, math2d::CoordinatedPoint{20, 20, coordinate_system});
    CHECK(reference_object.has_value());
    const auto skeleton_angle_step = 10;
    const auto nb_parts_of_object = 200;
    const auto reference_object_trace = deduct::ObjectTrace{
        *reference_object,
        deduct::SkeletonParams{
            skeleton_angle_step, nb_parts_of_object}}.get_trace();

    // Arrange image to scan
    const auto objects_data = generate_object_data();
    const auto test_image =
        create_test_image_with_objects(objects_data, 250, 250);

    // Act
    const auto objects = deduce_all_objects(executor, test_image);
    size_t count_similar_objects = 0;
    size_t i = 0;
    std::cout << "Squares" << std::endl;
    for (const auto object : objects) {
      const auto object_trace = deduct::ObjectTrace{
          object,
          deduct::SkeletonParams{
              skeleton_angle_step, nb_parts_of_object}}.get_trace();
      std::cout << "Object trace" << i++ << ": "  << object_trace.to_string() << std::endl;
      std::cout << "Reference trace: " << reference_object_trace.to_string() << std::endl;
      const auto same_outer = reference_object_trace.compare(
          object_trace, deduct::ComparisonParams{0.1, true, true, 0.8});
      if (same_outer) {
        count_similar_objects++;
      }
    }

    CHECK(count_similar_objects == 4);
  }

  SECTION("TraceDetectsTheCorrectNumberOfSimilarCircles") {
    // Arrange
    auto executor = par::Executor(1);
    const auto reference_circle = math2d::Circle{math2d::Point{15, 15}, 5};
    auto reference_objects = ObjectsData{};
    reference_objects.circles.push_back(
        ColoredObject{reference_circle, std::string("red"), 0});
    const auto reference_image =
        create_test_image_with_objects(reference_objects, 50, 50);
    const auto coordinate_system = math2d::CoordinateSystem{
        math2d::Point{0, 0}, math2d::Vector{1, 0}, math2d::Vector{0, 1}};
    const auto reference_object = deduce_object_at_position(
        executor, reference_image, math2d::CoordinatedPoint{15, 15, coordinate_system});
    CHECK(reference_object.has_value());
    const auto skeleton_angle_step = 10;
    const auto nb_parts_of_object = 200;
    const auto reference_object_trace = deduct::ObjectTrace{
        *reference_object,
        deduct::SkeletonParams{
            skeleton_angle_step, nb_parts_of_object}}.get_trace();

    // Arrange image to scan
    const auto objects_data = generate_object_data();
    const auto test_image =
        create_test_image_with_objects(objects_data, 250, 250);

    // Act
    const auto objects = deduce_all_objects(executor, test_image);
    size_t count_similar_objects = 0;
    size_t i = 0;
    std::cout << "Circles" << std::endl;
    for (const auto object : objects) {
      const auto object_trace = deduct::ObjectTrace{
          object,
          deduct::SkeletonParams{
              skeleton_angle_step, nb_parts_of_object}}.get_trace();
      std::cout << "Object trace" << i++ << ": "  << object_trace.to_string() << std::endl;
      std::cout << "Reference trace: " << reference_object_trace.to_string() << std::endl;
      const auto same_outer = reference_object_trace.compare(
          object_trace, deduct::ComparisonParams{0.1, true, true, 0.8});
      if (same_outer) {
        count_similar_objects++;
      }
    }

    CHECK(count_similar_objects == 5);
  }

  SECTION("TraceDetectsTheCorrectNumberOfSimilarRectangles") {
    // Arrange
    auto executor = par::Executor(1);
    const auto reference_rectangle =
        math2d::Rectangle{math2d::Point{15, 15}, math2d::Point{25, 35}};
    auto reference_objects = ObjectsData{};
    reference_objects.rectangles.push_back(
        ColoredObject{reference_rectangle, std::string("red"), 0});
    const auto reference_image =
        create_test_image_with_objects(reference_objects, 50, 50);
    const auto coordinate_system = math2d::CoordinateSystem{
        math2d::Point{0, 0}, math2d::Vector{1, 0}, math2d::Vector{0, 1}};
    const auto reference_object = deduce_object_at_position(
        executor, reference_image, math2d::CoordinatedPoint{20, 20, coordinate_system});
    CHECK(reference_object.has_value());
    const auto skeleton_angle_step = 10;
    const auto nb_parts_of_object = 200;
    const auto reference_object_trace = deduct::ObjectTrace{
        *reference_object,
        deduct::SkeletonParams{
            skeleton_angle_step, nb_parts_of_object}}.get_trace();

    // Arrange image to scan
    const auto objects_data = generate_object_data();
    const auto test_image =
        create_test_image_with_objects(objects_data, 250, 250);

    // Act
    const auto objects = deduce_all_objects(executor, test_image);
    size_t count_similar_objects = 0;
    size_t i = 0;
    std::cout << "Rectangles" << std::endl;
    for (const auto object : objects) {
      const auto object_trace = deduct::ObjectTrace{
          object,
          deduct::SkeletonParams{
              skeleton_angle_step, nb_parts_of_object}}.get_trace();
      std::cout << "Object trace" << i++ << ": "  << object_trace.to_string() << std::endl;
      std::cout << "Reference trace: " << reference_object_trace.to_string() << std::endl;
      const auto same_outer = reference_object_trace.compare(
          object_trace, deduct::ComparisonParams{0.1, true, true, 0.8});
      if (same_outer) {
        count_similar_objects++;
      }
    }

    CHECK(count_similar_objects == 3);
  }
}

} // namespace