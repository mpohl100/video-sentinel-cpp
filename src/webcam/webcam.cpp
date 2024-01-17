#include "webcam.h"

#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>

namespace webcam {

VideoCollector::VideoCollector(const std::string &path,
                               const std::string &appendedStr,
                               const cv::VideoCapture &input_cap)
    : _output_edges{get_video_name(path, appendedStr),
                    static_cast<int>(input_cap.get(cv::CAP_PROP_FOURCC)),
                    input_cap.get(cv::CAP_PROP_FPS),
                    cv::Size(input_cap.get(cv::CAP_PROP_FRAME_WIDTH),
                             input_cap.get(cv::CAP_PROP_FRAME_HEIGHT))} {
  if (!_output_edges.isOpened()) {
    std::cout << "!!! Output video edgescould not be opened" << std::endl;
    throw std::runtime_error("Cannot open output video for edges");
  }
}

VideoCollector::~VideoCollector() { _output_edges.release(); }

void VideoCollector::feed(const cv::Mat &frame) { _output_edges.write(frame); }

std::string VideoCollector::get_video_name(const std::string &path,
                                           const std::string &appendedStr) {
  // Find the position of the last '/' character in the path
  size_t lastSlashPos = path.find_last_of('/');

  // Extract the <my_name> part from the path
  std::string myName =
      path.substr(lastSlashPos + 1, path.find_last_of('.') - lastSlashPos - 1);

  // Construct the new video name by appending _<appendedStr> to <my_name>
  std::string newVideoName =
      path.substr(0, lastSlashPos + 1) + myName + "_" + appendedStr + ".mp4";

  return newVideoName;
};

void read_image_data(cv::VideoCapture &cap, cv::Mat &imgOriginal,
                     int &retflag) {
  retflag = 1;
  bool bSuccess = cap.read(imgOriginal); // read a new frame from video
  if (!bSuccess)                         // if not success, break loop
  {
    std::cout << "Cannot read a frame from video stream" << std::endl;
    {
      retflag = 2;
      return;
    };
  }
}

FrameData::FrameData(const cv::Mat &imgOriginal)
    : contours{imgOriginal.clone()}, gradient{imgOriginal.clone()},
      smoothed_contours_mat{imgOriginal.clone()},
      smoothed_gradient_mat{imgOriginal.clone()}, all_rectangles{} {}

par::Flow process_frame(FrameData &frame_data, const cv::Mat &imgOriginal,
                        const od::Rectangle &rectangle, int rings,
                        int gradient_threshold) {
  const auto create_flow = [&](const od::Rectangle &rectangle) {
    const auto calcGradient = [&, rectangle]() {
      std::cout << "calculating gradient" << std::endl;
      od::detect_directions(frame_data.gradient, imgOriginal, rectangle);
      std::cout << "gradient processed" << std::endl;
    };

    const auto calcSmoothedContours = [&, rectangle, rings,
                                       gradient_threshold]() {
      std::cout << "calculating smoothed contours" << std::endl;
      od::smooth_angles(frame_data.smoothed_contours_mat, frame_data.gradient,
                        rings, true, gradient_threshold, rectangle);
      std::cout << "smoothed contours processed" << std::endl;
    };
    [[maybe_unused]] const auto calcSmoothedGradient = [&]() {
      std::cout << "calculating smoothed gradient" << std::endl;
      od::smooth_angles(frame_data.smoothed_gradient_mat, frame_data.gradient,
                        rings, false, gradient_threshold, rectangle);
      std::cout << "smoothed gradient processed" << std::endl;
    };
    const auto calcAllRectangles = [&, rectangle]() {
      std::cout << "calculating all rectangles" << std::endl;
      od::establishing_shot_slices(frame_data.all_rectangles,
                                   frame_data.smoothed_contours_mat, rectangle);
      std::cout << "all rectangles processed" << std::endl;
    };

    auto flow = par::Flow{};
    // const auto calcCountoursTask = executor.emplace(calcCountours);
    flow.add(std::make_unique<par::Calculation>(calcGradient));
    flow.add(std::make_unique<par::Calculation>(calcSmoothedContours));
    // auto calcSmoothedGradientTask = taskflow.emplace(calcSmoothedGradient);
    flow.add(std::make_unique<par::Calculation>(calcAllRectangles));
    return flow;
  };

  return create_flow(rectangle);
}

std::vector<od::Rectangle> split_rectangle(const od::Rectangle &rectangle,
                                           int nb_splits) {
  const auto width = rectangle.width;
  const auto height = rectangle.height;
  const auto x = rectangle.x;
  const auto y = rectangle.y;
  const auto width_per_thread = width / nb_splits;
  const auto height_per_thread = height / nb_splits;
  auto rectangles = std::vector<od::Rectangle>{};
  for (auto i = 0; i < nb_splits; ++i) {
    for (auto j = 0; j < nb_splits; ++j) {
      rectangles.emplace_back(x + i * width_per_thread,
                              y + j * height_per_thread, width_per_thread,
                              height_per_thread);
    }
  }
  return rectangles;
}

FrameData process_frame_quadview(const cv::Mat &imgOriginal,
                                 const od::Rectangle &rectangle,
                                 par::Executor &executor, int rings,
                                 int gradient_threshold, int nb_splits) {
  auto frame_data = FrameData{imgOriginal};
  const auto rectangles = split_rectangle(rectangle, nb_splits);
  std::vector<par::Flow> flows;
  for (const auto &rect : rectangles) {
    flows.emplace_back(process_frame(frame_data, imgOriginal, rect, rings,
                                     gradient_threshold));
  }
  std::cout << "kicking off all tasks" << std::endl;
  for (auto &flow : flows) {
    std::cout << "kicking off task" << std::endl;
    executor.run(&flow);
  }
  for (auto &flow : flows) {
    executor.wait_for(&flow);
  }
  return frame_data;
}

FrameData process_frame_merged(const cv::Mat &imgOriginal,
                               const od::Rectangle &rectangle,
                               par::Executor &executor, int rings,
                               int gradient_threshold,
                               int nb_pixels_per_tile = 100) {
  auto frame_data = FrameData{imgOriginal};
  const auto rectangles = split_rectangle(rectangle, nb_pixels_per_tile);
  std::vector<par::Calculation> gradient_calculations;
  gradient_calculations.reserve(rectangles.size());
  std::vector<par::Task> gradient_tasks;
  gradient_tasks.reserve(rectangles.size());
  std::vector<par::Flow> smoothing_calculation_flows;
  smoothing_calculation_flows.reserve(rectangles.size());
  std::vector<par::Task> smoothing_tasks;
  smoothing_tasks.reserve(rectangles.size());

  for (const auto &rect : rectangles) {
    const auto calcGradient = [&, rect]() {
      std::cout << "calculating gradient" << std::endl;
      od::detect_directions(frame_data.gradient, imgOriginal, rect);
      std::cout << "gradient processed" << std::endl;
    };
    auto calculation = par::Calculation{calcGradient};
    gradient_calculations.emplace_back(calcGradient);
    gradient_tasks.emplace_back(calculation.make_task());
  }

  for (const auto &rect : rectangles){
    const auto calcSmoothedContours = [&, rect, rings,
                                       gradient_threshold]() {
      std::cout << "calculating smoothed contours" << std::endl;
      od::smooth_angles(frame_data.smoothed_contours_mat, frame_data.gradient,
                        rings, true, gradient_threshold, rect);
      std::cout << "smoothed contours processed" << std::endl;
    };
    const auto calcAllRectangles = [&, rect]() {
      std::cout << "calculating all rectangles" << std::endl;
      od::establishing_shot_slices(frame_data.all_rectangles,
                                   frame_data.smoothed_contours_mat, rect);
      std::cout << "all rectangles processed" << std::endl;
    };
    auto flow = par::Flow{};
    flow.add(std::make_unique<par::Calculation>(calcSmoothedContours));
    flow.add(std::make_unique<par::Calculation>(calcAllRectangles));
    smoothing_calculation_flows.emplace_back(flow);
    smoothing_tasks.emplace_back(flow.make_task());
  }

  // define dependencies between tasks
  size_t i = 0;
  for (const auto& rect : rectangles){
    std::vector<size_t> touching_rectangles = deduce_touching_rectangles(expand_rectangle(rect, rings), rectangles);
    for (const auto& touching_rectangle : touching_rectangles){
      smoothing_tasks[i].precede(gradient_tasks[touching_rectangle]);
    }
    i++;
  }

  // kick off tasks
  for (auto& gradient_calculation : gradient_calculations){
    executor.run(gradient_calculation);
  }
  for (auto& smoothing_calculation_flow : smoothing_calculation_flows){
    executor.run(&smoothing_calculation_flow);
  }

  for(auto& gradient_calculation : gradient_calculations){
    executor.wait_for(gradient_calculation);
  }
  for(auto& smoothing_calculation_flow : smoothing_calculation_flows){
    executor.wait_for(&smoothing_calculation_flow);
  }
  return frame_data;
}

} // namespace webcam