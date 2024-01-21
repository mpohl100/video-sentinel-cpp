#include "Slices.h"
#include "Object.h"

#include "math2d/math2d.h"

#include "Rectangle.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <vector>

namespace od {

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
    slices.slices.push_back(current_line);
  }
  return slices;
}

std::vector<Object> deduce_objects(Slices &slices) {
  std::vector<Object> objects;
  while (slices.contains_slices()) {
    std::vector<AnnotatedSlice> current_slices;
    const auto first_slice = slices.get_first_slice();
    current_slices.push_back(first_slice);
    auto current_object = Object{Slices{
        math2d::Point{first_slice.slice.start.x, first_slice.slice.start.y}}};
    current_object.slices.slices.push_back(current_slices);
    while (!current_slices.empty()) {
      current_slices = slices.get_touching_slices(current_slices);
      current_object.slices.slices.push_back(current_slices);
    }
    objects.push_back(current_object);
  }
  return objects;
}

AllRectangles deduce_rectangles(const std::vector<Object> &objects) {
  AllRectangles ret;
  for (const auto &object : objects) {
    auto rectangle = object.slices.to_rectangle();
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
      for (const auto &slice : slice_line) {
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
  const auto all_rectangles = deduce_rectangles(objects);
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);
  ret.rectangles.insert(ret.rectangles.end(), all_rectangles.rectangles.begin(),
                        all_rectangles.rectangles.end());
}

void print() { std::cout << "I am alive!" << std::endl; }

void establishing_slot_objects(ObjectsPerRectangle &ret,
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
      for (const auto &slice : slice_line) {
        std::cout << slice.slice.start.toString() << " "
                  << slice.slice.end.toString() << " | ";
      }
      std::cout << std::endl;
    }
    std::cout << "deducing objects ..." << std::endl;
  }
  const auto objects = deduce_objects(slices);
  ret.objects = objects;
  ret.rectangle = rectangle;
}

} // namespace od