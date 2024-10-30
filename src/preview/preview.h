#pragma once

#include "detection/AllObjects.h"
#include "par/parallel.h"
#include "webcam/webcam.h"

#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

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

  virtual ~VideoPreview() {
    bool do_wait = true;
    for(auto task : _current_task_graph.get_tasks()) {
      do_wait = do_wait && !_executor.does_not_know(task);
    }
    if (_frame_calculation_status == FrameCalculationStatus::IN_PROGRESS && do_wait) {
      _executor.wait_for(_current_task_graph);
    }
  }

  VideoPreview(size_t num_threads) : _executor(num_threads) {}

  void update_calculation_status() {
    bool is_done =
        _executor.wait_for(_current_task_graph, std::chrono::microseconds(0));
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

  virtual std::vector<od::Rectangle> get_all_rectangles() {
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
    _current_task_graph = webcam::process_frame_single_loop(_current_frame_data,
                                                            _current_original);
    adjust_task_graph(_current_task_graph);
    _executor.run(_current_task_graph);
  }

  virtual void adjust_task_graph([[maybe_unused]] par::TaskGraph &task_graph) {}

protected:
  par::Executor _executor;
  webcam::FrameData _current_frame_data;
  par::TaskGraph _current_task_graph;
private:
  cv::Mat _current_original;
  webcam::FrameData _processed_frame_data;
  FrameCalculationStatus _frame_calculation_status =
      FrameCalculationStatus::NOT_STARTED;
  RectanglesQueryStatus _rectangles_query_status =
      RectanglesQueryStatus::NOT_REQUESTED;
  std::mutex _processed_mutex;
};

} // namespace preview