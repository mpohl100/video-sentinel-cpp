#pragma once

#include "Rectangle.h"

#include "opencv2/core/mat.hpp"

#include <vector>

namespace od {

struct ObjectsPerRectangle;

struct Slice {
  math2d::Point start = math2d::Point{0, 0};
  math2d::Point end = math2d::Point{0, 0};

  friend constexpr auto operator<=>(const Slice &lhs,
                                    const Slice &rhs) = default;

  bool touches(const Slice &other) const {
    return (start >= other.start && start <= other.end) ||
           (end >= other.start && end <= other.end) ||
           (other.start >= start && other.start <= end) ||
           (other.end >= start && other.end <= end);
  }
};

struct AnnotatedSlice {
  Slice slice;
  size_t line_number = 0;
  friend constexpr auto operator<=>(const AnnotatedSlice &lhs,
                                    const AnnotatedSlice &rhs) = default;
};

struct Slices {
  std::vector<std::vector<AnnotatedSlice>> slices;
  math2d::Point top_left = math2d::Point{0, 0};

  Slices() = default;
  Slices(const Slices &) = default;
  Slices(Slices &&) = default;
  Slices &operator=(const Slices &) = default;
  Slices &operator=(Slices &&) = default;
  Slices(math2d::Point top_left) : top_left{top_left} {}

  bool contains_slices() const {
    for (const auto &slice_line : slices) {
      if (!slice_line.empty()) {
        return true;
      }
    }
    return false;
  }

  AnnotatedSlice get_first_slice() {
    for (auto &slice_line : slices) {
      if (!slice_line.empty()) {
        auto slice = slice_line.back();
        slice_line.pop_back();
        return slice;
      }
    }
    return AnnotatedSlice{};
  }

  std::vector<AnnotatedSlice>
  get_touching_slices(const std::vector<AnnotatedSlice> &slices_of_object) {
    if (slices_of_object.empty()) {
      return {};
    }
    const auto last_slice = slices_of_object.back();
    const auto line_number = last_slice.line_number;
    if (get_index(line_number) == slices.size() - 1) {
      return {};
    }
    const auto next_line_index = get_index(line_number + 1);
    auto &next_line = slices[next_line_index];
    std::vector<AnnotatedSlice> ret;
    for (const auto &annotatedSlice : slices_of_object) {
      for (const auto &slice : next_line) {
        if (annotatedSlice.slice.touches(slice.slice)) {
          ret.push_back(slice);
        }
      }
    }
    const auto last = std::unique(ret.begin(), ret.end());
    ret.erase(last, ret.end());
    std::sort(ret.begin(), ret.end());
    std::vector<AnnotatedSlice> cleared_next_line;
    std::set_difference(next_line.begin(), next_line.end(), ret.begin(),
                        ret.end(), std::back_inserter(cleared_next_line));
    slices[next_line_index] = cleared_next_line;
    return ret;
  }

  Rectangle to_rectangle() const {
    int min_x = 10000000;
    int max_x = 0;
    int min_y = 10000000;
    int max_y = 0;
    for (const auto &slice_line : slices) {
      for (const auto &slice : slice_line) {
        min_x = std::min(min_x, static_cast<int>(slice.slice.start.x));
        max_x = std::max(max_x, static_cast<int>(slice.slice.end.x));
        min_y = std::min(min_y, static_cast<int>(slice.slice.start.y));
        max_y = std::max(max_y, static_cast<int>(slice.slice.end.y));
      }
    }
    return Rectangle{min_x, min_y, max_x - min_x, max_y - min_y};
  }

  bool touching_right(const Rectangle &rectangle) const {
    for(const auto& slice : slices){
      if(slice.rbegin()->slice.end.x >= rectangle.top_left.x + rectangle.width - 1){
        return true;
      }
    }
    return false;
  }

  bool touching_down(const Rectangle &rectangle) const {
    if(slices.rbegin()->rbegin()->slice.start.y >= rectangle.top_left.y + rectangle.height - 1){
      return true;
    }
    return false;
  }

  bool touching_left(const Rectangle &rectangle) const {
    for(const auto &slice : slices){
      if(slice.begin()->slice.start.x <= rectangle.top_left.x){
        return true;
      }
    }
    return false;
  }

  bool touching_up(const Rectangle &rectangle) const {
    if(slices.begin()->begin()->slice.start.y <= rectangle.top_left.y){
      return true;
    }
    return false;
  }

  void merge_right(const Slices &other) {
    throw std::runtime_error("merge_right not implemented");
  } 

  void merge_down(const Slices &other) {
    throw std::runtime_error("merge_down not implemented");
  }
private:
  size_t get_index(size_t line_number) const {
    return line_number - top_left.y;
  }
};

struct AllRectangles {
  std::vector<Rectangle> rectangles;
};

void establishing_shot_slices(AllRectangles &ret, const cv::Mat &contours,
                              const Rectangle &rectangle);

void establishing_slot_objects(ObjectsPerRectangle &ret, const cv::Mat &contours,
                               const Rectangle &rectangle);
} // namespace od