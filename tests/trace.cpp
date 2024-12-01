#include <catch2/catch_all.hpp>

#include "deduction/ObjectTrace.h"
#include "math2d/math2d.h"
#include "webcam/webcam.h"

#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
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

template <typename T> struct ColoredObject {
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

TEST_CASE("Trace", "[trace]") {

  SECTION("TraceDetectsSquareInMultipleDirections") {
    auto executor = par::Executor(1);
    auto img0 = create_image_with_square(0);
    auto obj0 = deduce_object(executor, img0);
    CHECK(obj0.get_bounding_box().to_math2d_rectangle().area() == 100);
    const auto skeleton_params = deduct::SkeletonParams{30};
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
    const auto skeleton_params = deduct::SkeletonParams{30};
    auto object0_trace = deduct::ObjectTrace{obj0, skeleton_params}.get_trace();
    auto object1_trace = deduct::ObjectTrace{obj1, skeleton_params}.get_trace();

    const auto same_outer = object0_trace.compare(
        object1_trace, deduct::ComparisonParams{0.1, true});
    CHECK(same_outer);

    const auto same_inner = object0_trace.compare(
        object1_trace, deduct::ComparisonParams{0.1, false});
    CHECK(same_inner);

    const auto same_outer_integral = object0_trace.compare_integral(
        object1_trace, deduct::ComparisonParams{0.1, true});
    CHECK(same_outer);

    const auto same_inner_integral = object0_trace.compare_integral(
        object1_trace, deduct::ComparisonParams{0.1, false});
    CHECK(same_inner);
  }
}

} // namespace