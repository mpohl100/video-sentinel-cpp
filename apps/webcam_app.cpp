#include "detection/Detection.h"
#include "webcam/webcam.h"
#include "par/parallel.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <clara.hpp>

#include <iostream>
#include <stdexcept>

#define SINGLE_THREADED 0

int main(int argc, char **argv) {
  using namespace clara;

  int number_webcam = 0;
  int rings = 1;
  int gradient_threshold = 15;
  int rectangle_tl_x = 0;
  int rectangle_tl_y = 0;
  int rectangle_width = -1;
  int rectangle_height = -1;
  std::string path = "";
  bool short_run = false;
  bool help = false;
  auto cli =
      Opt(number_webcam, "number_webcam")["-n"]["--number-webcam"](
          "The number of the webcam to use") |
      Opt(path, "path")["-p"]["--path"]("The path to the video file") |
      Opt(rings, "rings")["-r"]["--rings"](
          "The number of rings to use for smoothing") |
      Opt(gradient_threshold, "threshold")["-t"]["--threshold"](
          "The threshold to use for smoothing") |
      Opt(rectangle_tl_x, "rectangle_tl_x")["-x"]["--rectangle-tl-x"](
          "The top left x coordinate") |
      Opt(rectangle_tl_y, "rectangle_tl_y")["-y"]["--rectangle-tl-y"](
          "The top left y coordinate") |
      Opt(rectangle_width,
          "rectangle_width")["-w"]["--rectangle-width"]("The rectangle width") |
      Opt(rectangle_height, "rectangle_height")["-h"]["--rectangle-height"](
          "The rectangle height") |
      Opt(short_run)["-s"]["--short-run"]("Run a short run") |
      Help(help);

  auto result = cli.parse(Args(argc, argv));
  if (!result) {
    std::cerr << "Error in command line: " << result.errorMessage() << '\n';
    exit(1);
  }
  if (help) {
    std::cout << cli;
    exit(0);
  }

  cv::VideoCapture cap;
  if (path != "") {
    cap = cv::VideoCapture{path};
    if (!cap.isOpened()) // if not success, exit program
    {
      std::cout << "Cannot open the video file" << std::endl;
      return -1;
    }
  } else {
    cap = cv::VideoCapture{number_webcam};
    if (!cap.isOpened()) // if not success, exit program
    {
      std::cout << "Cannot open the webcam" << std::endl;
      return -1;
    }
  } // capture the video from web cam

  auto collectorEdges = webcam::VideoCollector{path, "edge", cap};
  auto collectorSmoothed = webcam::VideoCollector{path, "smoothed", cap};
  auto collectorResult = webcam::VideoCollector{path, "result", cap};
  auto collectorGradientResult =
      webcam::VideoCollector{path, "result_gradient", cap};

  std::string original = "Original";
  std::string threshold = "Thresholded Image";
  std::string smoothed_angles = "Smoothed Angles";
  std::string smoothed_gradient = "Smoothed Gradient";
  // namedWindow(original, cv::WINDOW_AUTOSIZE);
  // namedWindow(threshold, cv::WINDOW_AUTOSIZE);
  // namedWindow(smoothed_angles, cv::WINDOW_AUTOSIZE);
  // namedWindow(smoothed_gradient, cv::WINDOW_AUTOSIZE);

  int i = 0;
  par::Executor executor(4);
  while (true) {
    cv::Mat imgOriginal;
    int retflag;
    webcam::read_image_data(cap, imgOriginal, retflag);
    auto rectangle = od::Rectangle{
        rectangle_tl_x, rectangle_tl_y,
        rectangle_width == -1 ? imgOriginal.cols : rectangle_width,
        rectangle_height == -1 ? imgOriginal.rows : rectangle_height};

    if (retflag == 2) {
      break;
    }
#if SINGLE_THREADED == 1
    auto frame_data = webcam::FrameData{imgOriginal};
    auto flow = webcam::process_frame(frame_data, imgOriginal, rectangle, rings, gradient_threshold);
    executor.run(flow);
    executor.wait_for(flow);
#else
    auto frame_data = webcam::process_frame_with_parallel_gradient(
        imgOriginal, rectangle, executor, rings, gradient_threshold);
#endif

    // draw all rectangles on copy of imgOriginal
    auto imgOriginalResult = imgOriginal.clone();
    for (const auto &rectangle : frame_data.all_rectangles.rectangles) {
      int rectX = std::max(0, rectangle.x);
      int rectY = std::max(0, rectangle.y);
      int rectWidth = std::min(imgOriginalResult.cols - rectX, rectangle.width);
      int rectHeight =
          std::min(imgOriginalResult.rows - rectY, rectangle.height);
      const auto cv_rectangle = cv::Rect{rectX, rectY, rectWidth, rectHeight};
      cv::rectangle(imgOriginalResult, cv_rectangle, cv::Scalar(0, 255, 0), 2);
    }

    auto imgGradientResult = frame_data.gradient.clone();
    for (const auto &rectangle : frame_data.all_rectangles.rectangles) {
      int rectX = std::max(0, rectangle.x);
      int rectY = std::max(0, rectangle.y);
      int rectWidth = std::min(imgGradientResult.cols - rectX, rectangle.width);
      int rectHeight =
          std::min(imgGradientResult.rows - rectY, rectangle.height);
      const auto cv_rectangle = cv::Rect{rectX, rectY, rectWidth, rectHeight};
      cv::rectangle(imgGradientResult, cv_rectangle, cv::Scalar(0, 255, 0), 2);
    }
    // imshow(threshold, contours);   // show the thresholded image
    // imshow(original, imgOriginal); // show the original image
    // imshow(smoothed_angles, smoothed_contours_mat);   // the smoothed
    // contours imshow(smoothed_gradient, smoothed_gradient_mat); // the
    // smoothed gradient

    collectorEdges.feed(frame_data.gradient);
    collectorSmoothed.feed(frame_data.smoothed_contours_mat);
    collectorResult.feed(imgOriginalResult);
    collectorGradientResult.feed(imgGradientResult);

    std::cout << "Frame " << ++i << " processed!" << std::endl;

    if(short_run && i > 10) {
      break;
    }

    if (cv::waitKey(30) == 27) // wait for 'esc' key press for 30ms. If 'esc'
                               // key is pressed, break loop
    {
      std::cout << "esc key is pressed by user" << std::endl;
      break;
    }
  }
  return 0;
}
