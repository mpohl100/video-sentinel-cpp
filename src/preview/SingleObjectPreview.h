#pragma once

#include "preview.h"

#include "deduction/ComparisonParams.h"
#include "deduction/ObjectTrace.h"
#include "deduction/SkeletonParams.h"
#include "deduction/Trace.h"

#include <optional>
#include <sstream>
#include <string>

namespace preview {

struct SingleObjectPreview : public VideoPreview {
  SingleObjectPreview(size_t num_threads, std::string ascii_art,
                      deduct::SkeletonParams skeleton_params,
                      deduct::ComparisonParams comparison_params)
      : VideoPreview(num_threads), _ascii_art{ascii_art},
        _skeleton_params{skeleton_params}, _comparison_params{
                                               comparison_params} {
    calculate_target();
  }

  virtual ~SingleObjectPreview(){
    bool do_wait = true;
    for(auto task : _current_task_graph.get_tasks()) {
      do_wait = do_wait && !_executor.does_not_know(task);
    }
    if (get_frame_calculation_status() == FrameCalculationStatus::IN_PROGRESS) {
      _executor.wait_for(_current_task_graph);
    }
  }

  void adjust_task_graph(par::TaskGraph &task_graph) override {
    const auto filter_objects = [this]() {
      const auto objects = _current_frame_data.result_objects.get_objects();
      _result_objects.clear();
      std::unique_lock<std::mutex> lock(_result_objects_mutex);
      for (const auto object : objects) {
        const auto trace =
            deduct::ObjectTrace{object, _skeleton_params}.get_trace();
        if (trace.compare(_target->trace, _comparison_params)) {
          _result_objects.push_back(object);
        }
      }
    };
    auto calc = par::Calculation{filter_objects};
    auto task = calc.make_task();
    for (auto &t : task_graph.get_added_tasks()) {
      task.succeed(t);
    }
    task_graph.add_task(task);
  }

  std::vector<od::Rectangle> get_all_rectangles() override {
    std::unique_lock<std::mutex> lock(_result_objects_mutex);
    std::vector<od::Rectangle> rectangles;
    for (const auto &object : _result_objects) {
      rectangles.push_back(object.get_bounding_box());
    }
    return rectangles;
  }

private:
  std::pair<size_t, size_t> deduce_dimensions(const std::string &ascii_art) {
    size_t rows = 0;
    size_t cols = 0;
    std::string line;
    auto sstream = std::istringstream{ascii_art};
    while (std::getline(sstream, line)) {
      rows++;
      cols = std::max(cols, line.size());
    }
    return {rows, cols};
  }

  void fill_mat(cv::Mat &mat, const std::string &ascii_art) {
    std::string line;
    size_t i = 0;
    auto sstream = std::istringstream{ascii_art};
    while (std::getline(sstream, line)) {
      for (size_t j = 0; j < line.size(); ++j) {
        if (line[j] == 'X' || line[j] == 'O') {
          mat.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
        }
      }
      i++;
    }
  }

  std::pair<size_t, size_t> find_target_pixel(const std::string &ascii_art) {
    size_t j = 0;
    std::string line;
    auto sstream = std::istringstream{ascii_art};
    while (std::getline(sstream, line)) {
      for (size_t k = 0; k < line.size(); ++k) {
        if (line[k] == 'O') {
          return {k, j};
        }
      }
      j++;
    }
    return {0, 0};
  }

  cv::Mat calculate_target_mat(const std::string &ascii_art) {
    const auto [rows, cols] = deduce_dimensions(ascii_art);
    cv::Mat img(rows, cols, CV_8UC3, cv::Scalar(255, 255, 255));
    fill_mat(img, ascii_art);
    return img;
  }

  void calculate_target() {
    const auto img = calculate_target_mat(_ascii_art);

    auto frame_data = webcam::FrameData{img};
    auto flow = webcam::process_frame_single_loop(frame_data, img);
    _executor.run(flow);
    _executor.wait_for(flow);
    auto objects = frame_data.result_objects.get_objects();

    const auto target_pixel = find_target_pixel(_ascii_art);
    for (const auto object : objects) {
      if (object.contains_point(math2d::Point{
              static_cast<math2d::number_type>(target_pixel.first),
              static_cast<math2d::number_type>(target_pixel.second)})) {
        _target = {object,
                   deduct::ObjectTrace{object, _skeleton_params}.get_trace()};
        break;
      }
    }
  }

  std::string _ascii_art;
  deduct::SkeletonParams _skeleton_params;
  deduct::ComparisonParams _comparison_params;

  struct Target {
    od::Object object;
    deduct::Trace trace;
  };

  std::optional<Target> _target;
  std::vector<od::Object> _result_objects;
  std::mutex _result_objects_mutex;
};

} // namespace preview