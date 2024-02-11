#pragma once

#include "Rectangle.h"

#include "opencv2/core/mat.hpp"

#include <optional>
#include <vector>

namespace od {

struct ObjectsPerRectangle;

struct Slice {
  math2d::Point start = math2d::Point{0, 0};
  math2d::Point end = math2d::Point{0, 0};

  friend constexpr auto operator<=>(const Slice &lhs,
                                    const Slice &rhs) = default;

  bool touches(const Slice &other) const {
    return (start.x >= other.start.x && start.x <= other.end.x) ||
           (end.x >= other.start.x && end.x <= other.end.x) ||
           (other.start.x >= start.x && other.start.x <= end.x) ||
           (other.end.x >= start.x && other.end.x <= end.x);
  }

  bool touches_with_tolerance(const Slice &other) const {
    constexpr int tolerance = 1;
    return (start.x >= (other.start.x - tolerance) &&
            start.x <= (other.end.x + tolerance)) ||
           (end.x >= (other.start.x - tolerance) &&
            end.x <= (other.end.x + tolerance)) ||
           (other.start.x >= (start.x - tolerance) &&
            other.start.x <= (end.x + tolerance)) ||
           (other.end.x >= (start.x - tolerance) &&
            other.end.x <= (end.x + tolerance));
  }
};

struct AnnotatedSlice {
  Slice slice;
  size_t line_number = 0;
  friend constexpr auto operator<=>(const AnnotatedSlice &lhs,
                                    const AnnotatedSlice &rhs) = default;
};

struct SliceLine {
  SliceLine(const SliceLine &) = default;
  SliceLine(SliceLine &&) = default;
  SliceLine &operator=(const SliceLine &) = default;
  SliceLine &operator=(SliceLine &&) = default;

  SliceLine(std::vector<AnnotatedSlice> line) : _line{std::move(line)} {
    if (!_line.empty()) {
      _line_number = _line.front().line_number;
    } else {
      throw std::runtime_error("can not deduce line number from empty line");
    }
  }

  SliceLine(std::vector<AnnotatedSlice> line, size_t line_number)
      : _line{std::move(line)}, _line_number{line_number} {
    for (const auto &slice : _line) {
      if (slice.line_number != line_number) {
        throw std::runtime_error("line number mismatch");
      }
    }
  }

  size_t line_number() const { return _line_number; }
  const std::vector<AnnotatedSlice> &line() const { return _line; }

  void merge_right(const SliceLine &other) {
    // if the linenumbers mismatch, throw
    if (line_number() != other.line_number()) {
      throw std::runtime_error(
          "Cannot merge slicelines with different line numbers");
    }
    // do the merging of the slicelines
    if (_line.back().slice.end.x >= other._line.front().slice.start.x) {
      _line.back().slice.end.x = other._line.front().slice.end.x;
    } else {
      _line.push_back(other._line.front());
    }
    for (size_t i = 1; i < other._line.size(); ++i) {
      _line.push_back(other._line[i]);
    }
  }

  bool does_overlap(const SliceLine &bottom) const {
    if (line().empty() || bottom.line().empty()) {
      return false;
    }
    if (line_number() + 1 != bottom.line_number()) {
      return false;
    }
    for (const auto &top_slice : line()) {
      for (const auto &bottom_slice : bottom.line()) {
        if (top_slice.slice.touches_with_tolerance(bottom_slice.slice)) {
          return true;
        }
      }
    }
    return false;
  }

  bool touches_rightmost_slice_or_is_nextto_it(const SliceLine &other) {
    if (_line.empty()) {
      return true;
    }
    if (other.line().empty()) {
      return true;
    }
    if (_line.back().slice.end.x < other.line().front().slice.start.x) {
      return true;
    }
    return false;
  }

  void pop_back() { _line.pop_back(); }

private:
  std::vector<AnnotatedSlice> _line;
  size_t _line_number = 0;
};

struct Slices {
  std::vector<SliceLine> slices;
  math2d::Point top_left = math2d::Point{0, 0};

  Slices() = default;
  Slices(const Slices &) = default;
  Slices(Slices &&) = default;
  Slices &operator=(const Slices &) = default;
  Slices &operator=(Slices &&) = default;
  Slices(math2d::Point top_left) : top_left{top_left} {}

  bool contains_slices() const {
    for (const auto &slice_line : slices) {
      if (!slice_line.line().empty()) {
        return true;
      }
    }
    return false;
  }

  std::optional<AnnotatedSlice> get_first_slice() {
    for (auto &slice_line : slices) {
      if (!slice_line.line().empty()) {
        auto slice = slice_line.line().back();
        slice_line.pop_back();
        return slice;
      }
    }
    return std::nullopt;
  }

  std::optional<SliceLine>
  get_touching_slices(const SliceLine &slices_of_object) {
    if (slices_of_object.line().empty()) {
      throw std::runtime_error("slices_of_object is empty");
    }
    const auto last_slice = slices_of_object.line().back();
    const auto line_number = slices_of_object.line_number();
    if (get_index(line_number) == slices.size() - 1) {
      return std::nullopt;
    }
    const auto next_line_index = get_index(line_number + 1);
    auto &next_line = slices[next_line_index];
    std::vector<AnnotatedSlice> ret;
    for (const auto &annotatedSlice : slices_of_object.line()) {
      for (const auto &slice : next_line.line()) {
        if (annotatedSlice.slice.touches(slice.slice)) {
          ret.push_back(slice);
        }
      }
    }
    const auto last = std::unique(ret.begin(), ret.end());
    ret.erase(last, ret.end());
    std::sort(ret.begin(), ret.end());
    std::vector<AnnotatedSlice> cleared_next_line;
    std::set_difference(next_line.line().begin(), next_line.line().end(),
                        ret.begin(), ret.end(),
                        std::back_inserter(cleared_next_line));
    slices[next_line_index] = SliceLine{cleared_next_line, line_number + 1};
    if (!ret.empty()) {
      return SliceLine{ret};
    }
    return std::nullopt;
  }

  Rectangle to_rectangle() const {
    int min_x = 10000000;
    int max_x = 0;
    int min_y = 10000000;
    int max_y = 0;
    for (const auto &slice_line : slices) {
      for (const auto &slice : slice_line.line()) {
        min_x = std::min(min_x, static_cast<int>(slice.slice.start.x));
        max_x = std::max(max_x, static_cast<int>(slice.slice.end.x));
        min_y = std::min(min_y, static_cast<int>(slice.slice.start.y));
        max_y = std::max(max_y, static_cast<int>(slice.slice.end.y));
      }
    }
    return Rectangle{min_x, min_y, max_x - min_x, max_y - min_y};
  }

  bool touching_right(const Rectangle &rectangle) const {
    for (const auto &sliceline : slices) {
      if (!sliceline.line().empty() && sliceline.line().rbegin()->slice.end.x >=
                                           rectangle.x + rectangle.width - 1) {
        return true;
      }
    }
    return false;
  }

  bool touching_down(const Rectangle &rectangle) const {
    if (!slices.empty() && slices.rbegin()->line_number() >=
                               (rectangle.y + rectangle.height - 1)) {
      return true;
    }
    return false;
  }

  bool touching_left(const Rectangle &rectangle) const {
    for (const auto &sliceline : slices) {
      if (!sliceline.line().empty() &&
          sliceline.line().begin()->slice.start.x <= rectangle.x) {
        return true;
      }
    }
    return false;
  }

  bool touching_up(const Rectangle &rectangle) const {
    if (!slices.empty() && !slices.begin()->line().empty() &&
        slices.begin()->line().begin()->slice.start.y <= rectangle.y) {
      return true;
    }
    return false;
  }

  bool touching_right(Slices &other) {
    const auto overlapping_lines = get_slices_on_the_same_line(other);
    for (const auto &[this_slice, other_slice] : overlapping_lines) {
      if (this_slice && other_slice) {
        if (this_slice->line().rbegin()->slice.end.x + 1 ==
            other_slice->line().begin()->slice.start.x) {
          return true;
        }
      }
    }
    return false;
  }

  void merge_right(Slices &other) {
    const auto overlapping_lines = get_slices_on_the_same_line(other);
    std::vector<SliceLine> new_slices;
    for (const auto &[this_slice, other_slice] : overlapping_lines) {
      if (this_slice && other_slice) {
        this_slice->merge_right(*other_slice);
        new_slices.push_back(*this_slice);
      } else if (this_slice) {
        new_slices.push_back(*this_slice);
      } else if (other_slice) {
        new_slices.push_back(*other_slice);
      }
    }
    slices = std::move(new_slices);
  }

  bool touching_down(const Slices &other) {
    const auto &other_first_line = other.slices.front();
    auto this_upper_line =
        get_line_by_number(other_first_line.line_number() - 1);
    if (!this_upper_line.has_value()) {
      return false;
    }
    if (!this_upper_line->does_overlap(other_first_line)) {
      return false;
    }
    // check that all lines of other only overlap at most with the right most
    // slice of this line
    auto current_line_number = other_first_line.line_number();
    size_t index = 1;
    while (index < other.slices.size()) {
      auto this_line = get_line_by_number(current_line_number);
      if (!this_line.has_value()) {
        break;
      }
      if (!this_line->touches_rightmost_slice_or_is_nextto_it(
              other.slices[index])) {
        return false;
      }
      ++current_line_number;
      ++index;
    }
    return true;
  }

  void merge_down(Slices &other) {
    if (!touching_down(other)) {
      return;
    }
    merge_right(other);
  }

private:
  size_t get_index(size_t line_number) const {
    return line_number - top_left.y;
  }

  std::optional<SliceLine> get_line_by_number(size_t line_number) const {
    if (line_number < top_left.y) {
      return std::nullopt;
    }
    const auto index = get_index(line_number);
    if (index >= slices.size()) {
      return std::nullopt;
    }
    return slices[index];
  }

  std::vector<std::pair<SliceLine *, SliceLine *>>
  get_slices_on_the_same_line(Slices &other) {
    std::vector<std::pair<SliceLine *, SliceLine *>> ret;
    size_t line_number = slices.front().line_number();
    size_t other_line_number = other.slices.front().line_number();
    if (line_number < other_line_number) {
      auto it = std::find_if(slices.begin(), slices.end(),
                             [other_line_number](const auto &slice_line) {
                               return slice_line.line().front().line_number ==
                                      other_line_number;
                             });
      // add slices of this
      for (auto this_it = slices.begin(); this_it != it; ++this_it) {
        ret.push_back({&*this_it, nullptr});
      }
      // add overlapping slices
      auto other_it = other.slices.begin();
      while (it != slices.end() && other_it != other.slices.end()) {
        ret.push_back({&*it, &*other_it});
        ++it;
        ++other_it;
      }
      // add other slices
      for (auto final_other_it = other_it; final_other_it != other.slices.end();
           ++final_other_it) {
        ret.push_back({nullptr, &*final_other_it});
      }
    } else {
      auto other_it = std::find_if(
          other.slices.begin(), other.slices.end(),
          [line_number](const auto &slice_line) {
            return slice_line.line().front().line_number == line_number;
          });
      // add slices of other
      for (auto this_it = other.slices.begin(); this_it != other_it;
           ++this_it) {
        ret.push_back({nullptr, &*this_it});
      }
      // add overlapping slices
      auto it = slices.begin();
      while (other_it != other.slices.end() && it != slices.end()) {
        ret.push_back({&*it, &*other_it});
        ++it;
        ++other_it;
      }
      // add slices of this
      for (auto final_it = it; final_it != slices.end(); ++final_it) {
        ret.push_back({&*final_it, nullptr});
      }
    }
    return ret;
  }
};

struct AllRectangles {
  std::vector<Rectangle> rectangles;
};

AllRectangles deduce_rectangles(const ObjectsPerRectangle &objects);

void establishing_shot_slices(AllRectangles &ret, const cv::Mat &contours,
                              const Rectangle &rectangle);

void establishing_shot_objects(ObjectsPerRectangle &ret,
                               const cv::Mat &contours,
                               const Rectangle &rectangle);
} // namespace od