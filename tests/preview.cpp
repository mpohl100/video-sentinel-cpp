#include <catch2/catch_all.hpp>

#include "preview/preview.h"

#include <iostream>

namespace {

TEST_CASE("Preview", "[preview]") {

  SECTION("PreviewProcessesFrameAfterPreviousFrameIsDone") {
    int rings = 1;
    int gradient_threshold = 20;
    const auto path = "../video/BillardTakeoff.mp4";

    auto cap = cv::VideoCapture{path};
    if (!cap.isOpened()) {
      std::cout << "!!! Input video could not be opened" << std::endl;
      throw std::runtime_error("Cannot open input video");
    }
    CHECK(cap.isOpened());

    auto video_preview = preview::VideoPreview{1};
    bool did_frame_get_ready_at_least_once = false;
    for (size_t i = 0; i < 100; i++) {
      std::cout << "Frame nr " << i << std::endl;
      cv::Mat imgOriginal;
      int retflag;
      webcam::read_image_data(cap, imgOriginal, retflag);

      if (retflag == 2) {
        CHECK(false);
        break;
      }

      video_preview.update_calculation_status();
      const auto frame_calculation_status =
          video_preview.get_frame_calculation_status();
      std::cout << "Frame status: " << preview::to_string(frame_calculation_status) << std::endl;
      if (frame_calculation_status ==
              preview::FrameCalculationStatus::NOT_STARTED ||
          frame_calculation_status == preview::FrameCalculationStatus::DONE) {
        video_preview.set_mat(
            imgOriginal,
            od::Rectangle{0, 0, imgOriginal.cols, imgOriginal.rows}, rings,
            gradient_threshold);
        std::cout << "Frame set" << std::endl;
        if (frame_calculation_status == preview::FrameCalculationStatus::DONE) {
          const auto rectangles = video_preview.get_all_rectangles();
          CHECK(rectangles.size() > 500);
          did_frame_get_ready_at_least_once = true;
          std::cout << "Frame done" << std::endl;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
    }
    CHECK(did_frame_get_ready_at_least_once);
  }
}

} // namespace