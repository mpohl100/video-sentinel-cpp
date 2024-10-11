#include "Slices.h"
#include "Object.h"
#include "ObjectsPerRectangle.h"

#include "math2d/math2d.h"

#include "Rectangle.h"

#include "opencv2/imgproc.hpp"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <optional>
#include <vector>

namespace od {

int compute_smoothed_gradient(const cv::Mat &grayImage, int row, int col) {
  std::array<int, 9> gradients = {};
  const auto compute_gradient = [&](int r, int c) -> int {
    int tl = static_cast<int>(grayImage.at<uchar>(r - 1, c - 1));
    int tc = static_cast<int>(grayImage.at<uchar>(r - 1, c));
    int tr = static_cast<int>(grayImage.at<uchar>(r - 1, c + 1));
    int cl = static_cast<int>(grayImage.at<uchar>(r, c - 1));
    int cc = static_cast<int>(grayImage.at<uchar>(r, c));
    int cr = static_cast<int>(grayImage.at<uchar>(r, c + 1));
    int bl = static_cast<int>(grayImage.at<uchar>(r + 1, c - 1));
    int bc = static_cast<int>(grayImage.at<uchar>(r + 1, c));
    int br = static_cast<int>(grayImage.at<uchar>(r + 1, c + 1));
    float sqrt2 = 1.0 / std::sqrt(2.0);
    float grad_tl_br = br - tl;
    float grad_cl_cr = cr - cl;
    float grad_bl_tr = tr - bl;
    float grad_bc_tc = tc - bc;
    float grad_x = grad_cl_cr + grad_tl_br * sqrt2 + grad_bl_tr * sqrt2;
    float grad_y = -grad_bc_tc + grad_tl_br * sqrt2 - grad_bl_tr * sqrt2;
    float grad_total = std::sqrt(grad_x * grad_x + grad_y * grad_y);
    if constexpr (false) {
      float cosAlpha = grad_x / grad_total;
      float radians = std::acos(cosAlpha);
      float degrees = radians * (180.0 / 3.1415926);
      if (grad_y < 0)
        degrees *= -1.0;
      int degrees_ret = int(degrees);

      //return {static_cast<int>(grad_total), degrees_ret};
    }
    return static_cast<int>(grad_total);
  };

  gradients[0] = compute_gradient(row - 1, col - 1);
  gradients[1] = compute_gradient(row - 1, col);
  gradients[2] = compute_gradient(row - 1, col + 1);
  gradients[3] = compute_gradient(row, col - 1);
  gradients[4] = compute_gradient(row, col);
  gradients[5] = compute_gradient(row, col + 1);
  gradients[6] = compute_gradient(row + 1, col - 1);
  gradients[7] = compute_gradient(row + 1, col);
  gradients[8] = compute_gradient(row + 1, col + 1);

  int sumLen = 0;
  for (int i = 0; i < gradients.size(); i++) {
    sumLen += gradients[i];
  }
  return sumLen / 9; 
}

Slices deduce_slices_single_loop(const cv::Mat &rgbImage, const Rectangle &rectangle) {
  auto slices =
      Slices{math2d::Point{static_cast<math2d::number_type>(rectangle.x),
                           static_cast<math2d::number_type>(rectangle.y)}};

  // Convert the BGR image to Grayscale in the region of interest
  cv::Mat grayImage;
  cv::cvtColor(rgbImage, grayImage, cv::COLOR_RGB2GRAY);

  std::optional<AnnotatedSlice> current_slice = std::nullopt;
  for (int y = 2; y < grayImage.rows - 2; ++y) {
    auto current_line = std::vector<AnnotatedSlice>{};
    const auto emplace_current_slice = [&]() {
      if (current_slice.has_value()) {
        current_line.push_back(current_slice.value());
        current_slice = std::nullopt;
      }
    };
    for (int x = 2; x < grayImage.cols - 2; ++x) {
      const auto gradient_value = compute_smoothed_gradient(grayImage, y, x);
      const auto point = math2d::Point{static_cast<math2d::number_type>(x),
                                       static_cast<math2d::number_type>(y)};
      // if pixel value is below threshold
      if (gradient_value <= 15) {
        if (!current_slice) {
          current_slice =
              AnnotatedSlice{Slice{point, point}, static_cast<size_t>(y)};
        } else {
          current_slice->slice.end = point;
        }
      } else {
        emplace_current_slice();
      }
    }
    emplace_current_slice();
    slices.slices.push_back(SliceLine{current_line, static_cast<size_t>(y)});
  }
  return slices;
}

Slices deduce_slices(const cv::Mat &contours, const Rectangle &rectangle) {
  auto slices =
      Slices{math2d::Point{static_cast<math2d::number_type>(rectangle.x),
                           static_cast<math2d::number_type>(rectangle.y)}};
  std::optional<AnnotatedSlice> current_slice = std::nullopt;
  cv::Vec3b const *row = nullptr;
  for (int y = od::row_min(0, rectangle);
       y < od::row_max(contours.rows, rectangle); ++y) {
    auto current_line = std::vector<AnnotatedSlice>{};
    const auto emplace_current_slice = [&]() {
      if (current_slice.has_value()) {
        current_line.push_back(current_slice.value());
        current_slice = std::nullopt;
      }
    };
    row = contours.ptr<const cv::Vec3b>(y);
    for (int x = od::col_min(0, rectangle);
         x < od::col_max(contours.cols, rectangle); ++x) {
      const auto point = math2d::Point{static_cast<math2d::number_type>(x),
                                       static_cast<math2d::number_type>(y)};
      const auto current_pixel = row[x];
      const auto is_within_object = [](const cv::Vec3b &pixel) {
        return pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255;
      };
      // if pixel is white
      if (is_within_object(current_pixel)) {
        if (!current_slice) {
          current_slice =
              AnnotatedSlice{Slice{point, point}, static_cast<size_t>(y)};
        } else {
          current_slice->slice.end = point;
        }
      } else {
        emplace_current_slice();
      }
    }
    emplace_current_slice();
    slices.slices.push_back(SliceLine{current_line, static_cast<size_t>(y)});
  }
  return slices;
}

std::shared_ptr<Object> deduce_object(const AnnotatedSlice &first_slice,
                                      Slices &image_slices) {
  auto object_slices = Slices{first_slice.slice.start};
  // insert the next slice
  object_slices.slices.push_back(
      SliceLine{std::vector<AnnotatedSlice>{first_slice},
                static_cast<size_t>(first_slice.slice.start.y)});
  // the first direction one iterates is from top to bottom
  auto direction = Slices::Direction::DOWN;
  bool increased_object_slices = false;
  int i = 0;
  do {
    increased_object_slices =
        object_slices.try_add_image_slices(image_slices, direction);
    direction = object_slices.invert_direction(direction);
    i++;
  } while (increased_object_slices || i < 2);
  return std::make_shared<Object>(object_slices);
}

std::vector<std::shared_ptr<Object>> deduce_objects(Slices &slices) {
  std::vector<std::shared_ptr<Object>> objects;
  while (slices.contains_slices()) {
    const auto first_slice = slices.get_first_slice();
    if (!first_slice.has_value()) {
      break;
    }
    objects.push_back(deduce_object(first_slice.value(), slices));
  }
  return objects;
}

AllRectangles
deduce_rectangles(const ObjectsPerRectangle &objects_per_rectangle) {
  AllRectangles ret;
  for (const auto &object : objects_per_rectangle.get_objects()) {
    auto rectangle = object->slices.to_rectangle();
    const auto expanded_rectangle =
        Rectangle{rectangle.x - 5, rectangle.y - 5, rectangle.width + 10,
                  rectangle.height + 10};
    ret.rectangles.push_back(expanded_rectangle);
  }
  return ret;
}

void establishing_shot_slices(AllRectangles &ret, const cv::Mat &contours,
                              const Rectangle &rectangle) {
  constexpr auto debug = false;
  if constexpr (debug) {
    std::cout << "establishing_shot_slices" << std::endl;
    std::cout << "deducing slices ..." << std::endl;
  }
  auto slices = deduce_slices(contours, rectangle);
  if constexpr (debug) {
    std::cout << "slices: " << std::endl;
    for (const auto &slice_line : slices.slices) {
      for (const auto &slice : slice_line.line()) {
        std::cout << slice.slice.start.toString() << " "
                  << slice.slice.end.toString() << " | ";
      }
      std::cout << std::endl;
    }
    std::cout << "deducing objects ..." << std::endl;
  }
  const auto objects = deduce_objects(slices);
  if constexpr (debug) {
    std::cout << "deducing rectangles ..." << std::endl;
  }
  auto objects_per_rectangle = ObjectsPerRectangle{};
  objects_per_rectangle.set_rectangle(rectangle);
  for (const auto &object : objects) {
    objects_per_rectangle.insert_object(object);
  }
  const auto all_rectangles = deduce_rectangles(objects_per_rectangle);
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);
  ret.rectangles.insert(ret.rectangles.end(), all_rectangles.rectangles.begin(),
                        all_rectangles.rectangles.end());
}

void print() { std::cout << "I am alive!" << std::endl; }

void establishing_shot_objects(ObjectsPerRectangle &ret,
                               const cv::Mat &contours,
                               const Rectangle &rectangle) {
  constexpr auto debug = false;
  if constexpr (debug) {
    std::cout << "establishing_shot_slices" << std::endl;
    std::cout << "deducing slices ..." << std::endl;
  }
  auto slices = deduce_slices(contours, rectangle);
  if constexpr (debug) {
    std::cout << "slices: " << std::endl;
    for (const auto &slice_line : slices.slices) {
      for (const auto &slice : slice_line.line()) {
        std::cout << slice.slice.start.toString() << " "
                  << slice.slice.end.toString() << " | ";
      }
      std::cout << std::endl;
    }
    std::cout << "deducing objects ..." << std::endl;
  }
  auto objects = deduce_objects(slices);
  ret.set_rectangle(rectangle);
  for (const auto &object : objects) {
    ret.insert_object(object);
  }
}

void establishing_shot_single_loop(AllRectangles &ret, const cv::Mat &rgbImage,
                              const Rectangle &rectangle) {
  constexpr auto debug = false;
  if constexpr (debug) {
    std::cout << "establishing_shot_slices" << std::endl;
    std::cout << "deducing slices ..." << std::endl;
  }
  auto slices = deduce_slices_single_loop(rgbImage, rectangle);
  if constexpr (debug) {
    std::cout << "slices: " << std::endl;
    for (const auto &slice_line : slices.slices) {
      for (const auto &slice : slice_line.line()) {
        std::cout << slice.slice.start.toString() << " "
                  << slice.slice.end.toString() << " | ";
      }
      std::cout << std::endl;
    }
    std::cout << "deducing objects ..." << std::endl;
  }
  const auto objects = deduce_objects(slices);
  if constexpr (debug) {
    std::cout << "deducing rectangles ..." << std::endl;
  }
  auto objects_per_rectangle = ObjectsPerRectangle{};
  objects_per_rectangle.set_rectangle(rectangle);
  for (const auto &object : objects) {
    objects_per_rectangle.insert_object(object);
  }
  const auto all_rectangles = deduce_rectangles(objects_per_rectangle);
  static std::mutex single_loop_mutex;
  std::lock_guard<std::mutex> lock(single_loop_mutex);
  ret.rectangles.insert(ret.rectangles.end(), all_rectangles.rectangles.begin(),
                        all_rectangles.rectangles.end());
}


} // namespace od