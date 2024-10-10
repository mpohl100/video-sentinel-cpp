#pragma once

#include "detection/AllObjects.h"
#include "detection/Detection.h"
#include "detection/Object.h"
#include "detection/ObjectsPerRectangle.h"
#include "detection/Slices.h"

#include "par/parallel.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <string>

namespace webcam {

class VideoCollector {
public:
  VideoCollector(const std::string &path, const std::string &appendedStr,
                 const cv::VideoCapture &input_cap);

  ~VideoCollector();

  void feed(const cv::Mat &frame);

private:
  std::string get_video_name(const std::string &path,
                             const std::string &appendedStr);
  // members
  cv::VideoWriter _output_edges;
};

void read_image_data(cv::VideoCapture &cap, cv::Mat &imgOriginal, int &retflag);

struct FrameData {
  cv::Mat contours;
  cv::Mat gradient;
  cv::Mat smoothed_contours_mat;
  cv::Mat smoothed_gradient_mat;
  od::AllObjects all_objects;
  od::ObjectsPerRectangle result_objects;
  od::AllRectangles all_rectangles;

  FrameData() = default;
  FrameData(const FrameData &) = delete;
  FrameData(FrameData &&) = default;
  FrameData &operator=(const FrameData &) = delete;
  FrameData &operator=(FrameData &&) = default;

  FrameData(const cv::Mat &imgOriginal);
};

par::Task process_frame(FrameData &frameData, const cv::Mat &imgOriginal,
                        const od::Rectangle &rectangle, int rings,
                        int gradient_threshold);

par::Task process_frame_single_loop(FrameData &frameData,
                                    const cv::Mat &imgOriginal);

FrameData process_frame_quadview(const cv::Mat &imgOriginal,
                                 const od::Rectangle &rectangle,
                                 par::Executor &executor, int rings,
                                 int gradient_threshold, int nb_splits = 2);

FrameData process_frame_merged(const cv::Mat &imgOriginal,
                               const od::Rectangle &rectangle,
                               par::Executor &executor, int rings,
                               int gradient_threshold,
                               int nb_pixels_per_tile = 100);

FrameData process_frame_merge_objects(const cv::Mat &imgOriginal,
                                      const od::Rectangle &rectangle,
                                      par::Executor &executor, int rings,
                                      int gradient_threshold,
                                      int nb_pixels_per_tile = 100);

par::TaskGraph process_frame_with_parallel_gradient(
    FrameData &frame_data, const cv::Mat &imgOriginal,
    const od::Rectangle &rectangle, int rings, int gradient_threshold,
    int nb_pixels_per_tile = 100);

} // namespace webcam