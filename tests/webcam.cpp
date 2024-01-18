#include <catch2/catch_all.hpp>

#include "webcam/webcam.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

namespace {

TEST_CASE("Webcam", "[webcam]") {

  SECTION("WebcamProcessFrame") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 20;
    const auto path = "../video/BillardTakeoff.mp4";

    auto cap = cv::VideoCapture{path};
    if (!cap.isOpened()) {
      std::cout << "!!! Input video could not be opened" << std::endl;
      throw std::runtime_error("Cannot open input video");
    }
    CHECK(cap.isOpened());

    cv::Mat imgOriginal;
    int retflag = -1;
    webcam::read_image_data(cap, imgOriginal, retflag);

    CHECK(retflag != 2);
    if (retflag == 2) {
      return;
    }

    const auto rectangle =
        od::Rectangle{0, 0, imgOriginal.cols, imgOriginal.rows};
    auto frame_data = webcam::FrameData{imgOriginal};
    auto flow = webcam::process_frame(frame_data, imgOriginal, rectangle, rings,
                                      gradient_threshold);
    executor.run(flow);
    executor.wait_for(flow);

    CHECK(frame_data.all_rectangles.rectangles.size() == 3335);
  }

  SECTION("WebcamProcessFrameQuadView") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 20;
    const auto path = "../video/BillardTakeoff.mp4";

    auto cap = cv::VideoCapture{path};
    if (!cap.isOpened()) {
      std::cout << "!!! Input video could not be opened" << std::endl;
      throw std::runtime_error("Cannot open input video");
    }
    CHECK(cap.isOpened());

    cv::Mat imgOriginal;
    int retflag = -1;
    webcam::read_image_data(cap, imgOriginal, retflag);

    CHECK(retflag != 2);
    if (retflag == 2) {
      return;
    }

    const auto rectangle =
        od::Rectangle{0, 0, imgOriginal.cols, imgOriginal.rows};
    const auto frame_data = webcam::process_frame_quadview(
        imgOriginal, rectangle, executor, rings, gradient_threshold);

    CHECK(frame_data.all_rectangles.rectangles.size() == 3335);
  }
#if 1
  SECTION("WebcamProcessFrameManyTasks") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 20;
    const auto path = "../video/BillardTakeoff.mp4";

    auto cap = cv::VideoCapture{path};
    if (!cap.isOpened()) {
      std::cout << "!!! Input video could not be opened" << std::endl;
      throw std::runtime_error("Cannot open input video");
    }
    CHECK(cap.isOpened());

    cv::Mat imgOriginal;
    int retflag = -1;
    webcam::read_image_data(cap, imgOriginal, retflag);

    CHECK(retflag != 2);
    if (retflag == 2) {
      return;
    }

    const auto rectangle =
        od::Rectangle{0, 0, imgOriginal.cols, imgOriginal.rows};
    const auto frame_data = webcam::process_frame_merged(
        imgOriginal, rectangle, executor, rings, gradient_threshold);

    CHECK(frame_data.all_rectangles.rectangles.size() == 3335);
  }
#endif
}

} // namespace