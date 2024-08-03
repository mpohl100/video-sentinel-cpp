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

par::Task process_frame(FrameData &frame_data, const cv::Mat &imgOriginal,
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
    flow.add(par::Calculation{calcGradient});
    flow.add(par::Calculation{calcSmoothedContours});
    // auto calcSmoothedGradientTask = taskflow.emplace(calcSmoothedGradient);
    flow.add(par::Calculation{calcAllRectangles});
    return flow.make_task();
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
  std::vector<par::Task> flows;
  for (const auto &rect : rectangles) {
    flows.emplace_back(process_frame(frame_data, imgOriginal, rect, rings,
                                     gradient_threshold));
  }
  std::cout << "kicking off all tasks" << std::endl;
  for (auto &flow : flows) {
    std::cout << "kicking off task" << std::endl;
    executor.run(flow);
  }
  for (auto &flow : flows) {
    executor.wait_for(flow);
  }
  return frame_data;
}

od::Rectangle expand_rectangle(const od::Rectangle &rectangle, int rings) {
  return od::Rectangle{rectangle.x - rings, rectangle.y - rings,
                       rectangle.width + 2 * rings,
                       rectangle.height + 2 * rings};
}

std::vector<size_t>
deduce_touching_rectangles(const od::Rectangle &rectangle,
                           const std::vector<od::Rectangle> &rectangles) {
  std::vector<size_t> touching_rectangles;
  touching_rectangles.reserve(9);
  size_t i = 0;
  const auto target_rect = rectangle.to_math2d_rectangle();
  for (const auto &rect : rectangles) {
    const auto current_math_rect = rect.to_math2d_rectangle();
    if (current_math_rect.intersects(target_rect) || rectangle.contains(rect)) {
      touching_rectangles.emplace_back(i);
    }
    i++;
  }
  return touching_rectangles;
}

std::vector<od::Rectangle>
split_rectangle_into_parts(const od::Rectangle &rectangle,
                           int nb_pixels_per_tile) {
  constexpr auto debug = true;
  std::vector<od::Rectangle> rectangles;
  for (size_t y = rectangle.y; y < rectangle.y + rectangle.height;
       y += nb_pixels_per_tile) {
    for (size_t x = rectangle.x; x < rectangle.x + rectangle.width;
         x += nb_pixels_per_tile) {
      rectangles.emplace_back(x, y, nb_pixels_per_tile, nb_pixels_per_tile);
    }
  }
  if(debug){
    std::cout << "Rectangles: " << std::endl;
    for(const auto& rect : rectangles){
      std::cout << "x: " << rect.x << " y: " << rect.y << " width: " << rect.width << " height: " << rect.height << std::endl;
    }
  }
  return rectangles;
}

FrameData process_frame_merged(const cv::Mat &imgOriginal,
                               const od::Rectangle &rectangle,
                               par::Executor &executor, int rings,
                               int gradient_threshold, int nb_pixels_per_tile) {
  constexpr auto debug = true;
  auto frame_data = FrameData{imgOriginal};
  const auto rectangles =
      split_rectangle_into_parts(rectangle, nb_pixels_per_tile);
  std::vector<par::Task> gradient_tasks;
  gradient_tasks.reserve(rectangles.size());
  std::vector<par::Task> smoothing_tasks;
  smoothing_tasks.reserve(rectangles.size());

  for (const auto &rect : rectangles) {
    const auto calcGradient = [&, rect]() {
      if (debug)
        std::cout << "calculating gradient for rect " << rect.to_string()
                  << std::endl;
      od::detect_directions(frame_data.gradient, imgOriginal, rect);
      if (debug)
        std::cout << "gradient processedfor rect " << rect.to_string()
                  << std::endl;
    };
    auto calculation = par::Calculation{calcGradient};
    gradient_tasks.emplace_back(calculation.make_task());
  }

  for (const auto &rect : rectangles) {
    const auto calcSmoothedContours = [&, rect, rings, gradient_threshold]() {
      if (debug)
        std::cout << "calculating smoothed contours for rect "
                  << rect.to_string() << std::endl;
      od::smooth_angles(frame_data.smoothed_contours_mat, frame_data.gradient,
                        rings, true, gradient_threshold, rect);
      if (debug)
        std::cout << "smoothed contours processed for rect " << rect.to_string()
                  << std::endl;
    };
    const auto calcAllRectangles = [&, rect]() {
      if (debug)
        std::cout << "calculating all rectangles for rect " << rect.to_string()
                  << std::endl;
      od::establishing_shot_slices(frame_data.all_rectangles,
                                   frame_data.smoothed_contours_mat, rect);
      if (debug)
        std::cout << "all rectangles processed for rect " << rect.to_string()
                  << std::endl;
    };
    auto flow = par::Flow{};
    flow.add(par::Calculation{calcSmoothedContours});
    flow.add(par::Calculation{calcAllRectangles});

    smoothing_tasks.emplace_back(flow.make_task());
  }

  // define dependencies between tasks
  size_t i = 0;
  for (const auto &rect : rectangles) {
    std::vector<size_t> touching_rectangles =
        deduce_touching_rectangles(expand_rectangle(rect, rings), rectangles);
    for (const auto &touching_rectangle : touching_rectangles) {
      smoothing_tasks[i].succeed(gradient_tasks[touching_rectangle]);
    }
    i++;
  }

  // kick off tasks
  for (auto &gradient_task : gradient_tasks) {
    executor.run(gradient_task);
  }
  for (auto &smoothing_task : smoothing_tasks) {
    executor.run(smoothing_task);
  }

  for (auto &gradient_task : gradient_tasks) {
    executor.wait_for(gradient_task);
  }
  for (auto &smoothing_task : smoothing_tasks) {
    executor.wait_for(smoothing_task);
  }
  return frame_data;
}

FrameData process_frame_merge_objects(const cv::Mat &imgOriginal,
                                      const od::Rectangle &rectangle,
                                      par::Executor &executor, int rings,
                                      int gradient_threshold,
                                      int nb_pixels_per_tile) {
  constexpr auto debug = true;
  auto frame_data = FrameData{imgOriginal};
  const auto rectangles =
      split_rectangle_into_parts(rectangle, nb_pixels_per_tile);
  std::vector<par::Task> gradient_tasks;
  gradient_tasks.reserve(rectangles.size());
  std::vector<par::Task> smoothing_tasks;
  smoothing_tasks.reserve(rectangles.size());

  for (const auto &rect : rectangles) {
    const auto calcGradient = [&, rect]() {
      if (debug)
        std::cout << "calculating gradient for rect " << rect.to_string()
                  << std::endl;
      od::detect_directions(frame_data.gradient, imgOriginal, rect);
      if (debug)
        std::cout << "gradient processedfor rect " << rect.to_string()
                  << std::endl;
    };
    auto calculation = par::Calculation{calcGradient};
    gradient_tasks.emplace_back(calculation.make_task());
  }

  // print all rectangles:
  if constexpr (debug) {
    for (const auto &rect : rectangles) {
      std::cout << "rect: " << rect.to_string() << std::endl;
    }
  }

  frame_data.all_objects = od::AllObjects{
      static_cast<size_t>(rectangle.height / nb_pixels_per_tile) + 1,
      static_cast<size_t>(rectangle.width / nb_pixels_per_tile) + 1};
  for (const auto &rect : rectangles) {
    const auto calcSmoothedContours = [&, rect, rings, gradient_threshold]() {
      if (debug)
        std::cout << "calculating smoothed contours for rect "
                  << rect.to_string() << std::endl;
      od::smooth_angles(frame_data.smoothed_contours_mat, frame_data.gradient,
                        rings, true, gradient_threshold, rect);
      if (debug)
        std::cout << "smoothed contours processed for rect " << rect.to_string()
                  << std::endl;
    };
    const auto calcAllObjects = [&, rect]() {
      if (debug)
        std::cout << "calculating all rectangles for rect " << rect.to_string()
                  << std::endl;
      od::establishing_shot_objects(
          frame_data.all_objects.get(rect.y / nb_pixels_per_tile,
                                     rect.x / nb_pixels_per_tile),
          frame_data.smoothed_contours_mat, rect);
      if (debug)
        std::cout << "all rectangles processed for rect " << rect.to_string()
                  << std::endl;
    };
    auto flow = par::Flow{};
    flow.add(par::Calculation{calcSmoothedContours});
    flow.add(par::Calculation{calcAllObjects});

    smoothing_tasks.emplace_back(flow.make_task());
  }

  // define dependencies between tasks
  size_t i = 0;
  for (const auto &rect : rectangles) {
    std::vector<size_t> touching_rectangles =
        deduce_touching_rectangles(expand_rectangle(rect, rings), rectangles);
    for (const auto &touching_rectangle : touching_rectangles) {
      smoothing_tasks[i].succeed(gradient_tasks[touching_rectangle]);
    }
    i++;
  }

  // kick off tasks
  for (auto &gradient_task : gradient_tasks) {
    executor.run(gradient_task);
  }
  for (auto &smoothing_task : smoothing_tasks) {
    executor.run(smoothing_task);
  }

  for (auto &gradient_task : gradient_tasks) {
    executor.wait_for(gradient_task);
  }
  for (auto &smoothing_task : smoothing_tasks) {
    executor.wait_for(smoothing_task);
  }
  if constexpr (debug) {
    // print all rectangles in the matrix
    for (size_t row = 0; row < frame_data.all_objects.get_rows(); ++row) {
      for (size_t col = 0; col < frame_data.all_objects.get_cols(); ++col) {
        std::cout << "row " << row << " col " << col << ": ";
        std::cout
            << frame_data.all_objects.get(row, col).get_rectangle().to_string()
            << std::endl;
        for(const auto& object : frame_data.all_objects.get(row, col).get_objects()){
          std::cout << "object: " << object->get_bounding_box().to_string() << std::endl;
        }
      }
    }
  }
  // merge all objects
  std::vector<par::Task> append_right_tasks;
  std::vector<od::ObjectsPerRectangle> line_objects;
  for (size_t row = 0; row < frame_data.all_objects.get_rows(); ++row) {
    line_objects.emplace_back(frame_data.all_objects.get(row, 0));
    auto flow = par::Flow{};
    for (size_t col = 1; col < frame_data.all_objects.get_cols(); ++col) {
      const auto append_right = [&, row, col]() {
        if(debug){
          std::cout << "Appending right row " << row << " col " << col << std::endl;
        }
        line_objects[row].append_right(frame_data.all_objects.get(row, col));
        if(debug)
          std::cout << "Finished appending right row " << row << " col " << col << std::endl;
      };
      flow.add(par::Calculation{append_right});
    }
    append_right_tasks.emplace_back(flow.make_task());
  }
  for (const auto &task : append_right_tasks) {
    executor.run(task);
  }
  for (const auto &task : append_right_tasks) {
    executor.wait_for(task);
  }
  frame_data.result_objects = line_objects[0];
  for (size_t i = 1; i < line_objects.size(); ++i) {
    if(debug)
      std::cout << "Appending down row " << i << std::endl;
    frame_data.result_objects.append_down(line_objects[i]);
    if(debug)
      std::cout << "Finished appending down row " << i << std::endl;
  }

  frame_data.all_rectangles = od::deduce_rectangles(frame_data.result_objects);

  return frame_data;
}

} // namespace webcam