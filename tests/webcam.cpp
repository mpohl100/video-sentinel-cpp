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
    int gradient_threshold = 15;
    const auto path = std::string(CMAKE_SRC_DIR) + "/video/BillardTakeoff.mp4";

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

    CHECK(frame_data.all_rectangles.rectangles.size() > 500);
  }

SECTION("WebcamProcessFrameSingleLoop") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 15;
    const auto path = std::string(CMAKE_SRC_DIR) + "/video/BillardTakeoff.mp4";

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
    auto flow = webcam::process_frame_single_loop(frame_data, imgOriginal);
    executor.run(flow);
    executor.wait_for(flow);

    CHECK(frame_data.all_rectangles.rectangles.size() > 500);
  }


  SECTION("WebcamProcessFrameQuadView") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 15;
    const auto path = std::string(CMAKE_SRC_DIR) + "/video/BillardTakeoff.mp4";

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
    const auto taskgraph = webcam::process_frame_quadview(
        frame_data, imgOriginal, rectangle);
    executor.run(taskgraph);
    executor.wait_for(taskgraph);

    CHECK(frame_data.all_rectangles.rectangles.size() > 500);
  }
  SECTION("WebcamProcessFrameManyTasks") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 15;
    const auto path = std::string(CMAKE_SRC_DIR) + "/video/BillardTakeoff.mp4";

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

    CHECK(frame_data.all_rectangles.rectangles.size() > 500);
  }
  SECTION("WebcamProcessFrameMergeObjects") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 15;
    const auto path = std::string(CMAKE_SRC_DIR) + "/video/BillardTakeoff.mp4";

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
    const auto frame_data = webcam::process_frame_merge_objects(
        imgOriginal, rectangle, executor, rings, gradient_threshold);

    CHECK(frame_data.all_rectangles.rectangles.size() > 500);
  }

  SECTION("WebcamProcessFrameMergeObjectsParallelizedGradient") {
    par::Executor executor(4);
    int rings = 1;
    int gradient_threshold = 15;
    const auto path = std::string(CMAKE_SRC_DIR) + "/video/BillardTakeoff.mp4";

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
    auto frame_task_graph = webcam::process_frame_with_parallel_gradient(
        frame_data, imgOriginal, rectangle, rings, gradient_threshold);
    executor.run(frame_task_graph);
    executor.wait_for(frame_task_graph);

    CHECK(frame_data.all_rectangles.rectangles.size() > 500);
  }

}

} // namespace