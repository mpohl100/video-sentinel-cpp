#include <catch2/catch_all.hpp>

#include "deduction/ObjectTrace.h"
#include "webcam/webcam.h"

#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

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

od::Object deduce_object(par::Executor& executor, const cv::Mat& img){
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

  SECTION("TwoSameObjectsOfDifferentImagesHaveTheSameForm"){
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
    
    const auto same_outer = object0_trace.compare(object1_trace, deduct::ComparisonParams{0.1, true});
    CHECK(same_outer);

    const auto same_inner = object0_trace.compare(object1_trace, deduct::ComparisonParams{0.1, false});
    CHECK(same_inner);
  }
}

} // namespace