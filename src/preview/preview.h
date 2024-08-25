#pragma once

#include "detection/AllObjects.h"
#include "par/parallel.h"
#include "webcam/webcam.h"

#include <optional>
#include <vector>
#include <iostream>

namespace preview {

enum FrameCalculationStatus { NOT_STARTED, IN_PROGRESS, DONE };

struct VideoPreview {
  VideoPreview() = default;
  VideoPreview(const VideoPreview &) = delete;
  VideoPreview(VideoPreview &&) = default;
  VideoPreview &operator=(const VideoPreview &) = delete;
  VideoPreview &operator=(VideoPreview &&) = default;

  ~VideoPreview() {
    if (_frame_calculation_status == FrameCalculationStatus::IN_PROGRESS) {
      _executor.wait_for(_current_task);
    }
  }

  VideoPreview(size_t num_threads) : _executor(num_threads) {}

  void update_calculation_status() {
    bool is_done =
        _executor.wait_for(_current_task, std::chrono::microseconds(0));
    if (is_done) {
      _processed_frame_data = std::move(_current_frame_data);
      _current_original.release();
      _frame_calculation_status = FrameCalculationStatus::DONE;
    }
  }

  FrameCalculationStatus get_frame_calculation_status() {
    return _frame_calculation_status;
  }

  std::vector<od::Rectangle> get_all_rectangles() const {
    return _processed_frame_data.all_rectangles.rectangles;
  }

  void set_mat(cv::Mat const &mat, od::Rectangle const &rectangle, int rings,
               int gradient_threshold) {
    std::cout << "setting mat" << std::endl;
    _frame_calculation_status = FrameCalculationStatus::IN_PROGRESS;
    _current_original = mat.clone();	
    _current_frame_data = webcam::FrameData{_current_original};
    _current_task = webcam::process_frame(_current_frame_data, _current_original,
                                          rectangle, rings, gradient_threshold);
    _executor.run(_current_task);
  }

private:
  par::Executor _executor;
  cv::Mat _current_original;
  webcam::FrameData _current_frame_data;
  webcam::FrameData _processed_frame_data;
  par::Task _current_task;
  FrameCalculationStatus _frame_calculation_status =
      FrameCalculationStatus::NOT_STARTED;
};

} // namespace preview