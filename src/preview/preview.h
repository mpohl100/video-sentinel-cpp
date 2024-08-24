#pragma once

#include "detection/AllObjects.h"
#include "par/parallel.h"
#include "webcam/webcam.h"

#include <optional>
#include <vector>

namespace preview {

enum FrameCalculationStatus { NOT_STARTED, IN_PROGRESS, DONE };

struct VideoPreview {
  VideoPreview() = default;
  VideoPreview(const VideoPreview &) = delete;
  VideoPreview(VideoPreview &&) = default;
  VideoPreview &operator=(const VideoPreview &) = delete;
  VideoPreview &operator=(VideoPreview &&) = default;
  VideoPreview(size_t num_threads) : _executor(num_threads) {}

  bool is_new_frame_ready() {
    if(_frame_calculation_status == FrameCalculationStatus::NOT_STARTED) {
      return false;
    }
    bool is_done =
        _executor.wait_for(_current_task, std::chrono::microseconds(0));
    if (is_done) {
      _processed_frame_data = std::move(_current_frame_data);
      _frame_calculation_status = FrameCalculationStatus::DONE;
      return true;
    }
    return false;
  }

  std::vector<od::Rectangle> get_all_rectangles() const {
    // this method should only be called after is_calculation_done() returns
    // true
    if (_frame_calculation_status != FrameCalculationStatus::DONE) {
      throw std::runtime_error("get_all_rectangles() called before "
                               "is_calculation_done() returned true");
    }
    return _processed_frame_data.all_rectangles.rectangles;
  }

  void set_mat(cv::Mat const &mat, od::Rectangle const &rectangle, int rings,
               int gradient_threshold) {
    // should only be set after is_calculation_done() returns true
    _frame_calculation_status = FrameCalculationStatus::IN_PROGRESS;
    _current_frame_data = webcam::FrameData{mat};
    _current_task = webcam::process_frame(_current_frame_data, _current_frame,
                                          rectangle, rings, gradient_threshold);
    _executor.run(_current_task);
  }

private:
  par::Executor _executor;
  cv::Mat _current_frame;
  webcam::FrameData _current_frame_data;
  webcam::FrameData _processed_frame_data;
  par::Task _current_task;
  FrameCalculationStatus _frame_calculation_status = FrameCalculationStatus::NOT_STARTED;
};

} // namespace preview