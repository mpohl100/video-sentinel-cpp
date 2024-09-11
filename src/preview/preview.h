#pragma once

#include "detection/AllObjects.h"
#include "par/parallel.h"
#include "webcam/webcam.h"

#include <iostream>
#include <mutex>
#include <optional>
#include <vector>
#include <string>

namespace preview {

enum class FrameCalculationStatus { NOT_STARTED, IN_PROGRESS, DONE };
enum class RectanglesQueryStatus { NOT_REQUESTED, REQUESTED };

inline std::string to_string(FrameCalculationStatus status) {
  switch (status) {
  case FrameCalculationStatus::NOT_STARTED:
    return "NOT_STARTED";
  case FrameCalculationStatus::IN_PROGRESS:
    return "IN_PROGRESS";
  case FrameCalculationStatus::DONE:
    return "DONE";
  }
  return "UNKNOWN";
}

struct VideoPreview {
  VideoPreview() = default;
  VideoPreview(const VideoPreview &) = delete;
  VideoPreview(VideoPreview &&) = delete;
  VideoPreview &operator=(const VideoPreview &) = delete;
  VideoPreview &operator=(VideoPreview &&) = delete;

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
      std::unique_lock<std::mutex> lock(_processed_mutex);
      _processed_frame_data = std::move(_current_frame_data);
      _current_original.release();
      _frame_calculation_status = FrameCalculationStatus::DONE;
    }
  }

  FrameCalculationStatus get_frame_calculation_status() {
    return _frame_calculation_status;
  }

  RectanglesQueryStatus get_rectangles_query_status() {
    return _rectangles_query_status;
  }

  std::vector<od::Rectangle> get_all_rectangles() {
    std::unique_lock<std::mutex> lock(_processed_mutex);
    _rectangles_query_status = RectanglesQueryStatus::REQUESTED;
    return _processed_frame_data.all_rectangles.rectangles;
  }

  void set_mat(cv::Mat const &mat, od::Rectangle const &rectangle, int rings,
               int gradient_threshold) {
    _frame_calculation_status = FrameCalculationStatus::IN_PROGRESS;
    _rectangles_query_status = RectanglesQueryStatus::NOT_REQUESTED;
    _current_original = mat.clone();
    _current_frame_data = webcam::FrameData{_current_original};
    _current_task =
        webcam::process_frame(_current_frame_data, _current_original, rectangle,
                              rings, gradient_threshold);
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
  RectanglesQueryStatus _rectangles_query_status =
      RectanglesQueryStatus::NOT_REQUESTED;
  std::mutex _processed_mutex;
};

} // namespace preview