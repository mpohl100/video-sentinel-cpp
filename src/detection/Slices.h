#pragma once

#include "Rectangle.h"

#include "opencv2/core/mat.hpp"

#include <optional>
#include <string>
#include <vector>

namespace od {

struct ObjectsPerRectangle;
struct Object;

struct Slice {
  math2d::Point start = math2d::Point{0, 0};
  math2d::Point end = math2d::Point{0, 0};

  // friend constexpr auto operator<=>(const Slice &lhs,
  //                                   const Slice &rhs) = default;
  friend inline bool operator<(const Slice &l, const Slice &r) {
    if (l.start != r.start) {
      return l.start < r.start;
    }
    return l.end < r.end;
  }

  friend inline bool operator==(const Slice &l, const Slice &r) {
    return !(l < r) && !(r < l);
  }

  friend inline bool operator!=(const Slice &l, const Slice &r) {
    return !(l == r);
  }

  bool touches(const Slice &other) const {
    return (start.x >= other.start.x && start.x <= other.end.x) ||
           (end.x >= other.start.x && end.x <= other.end.x) ||
           (other.start.x >= start.x && other.start.x <= end.x) ||
           (other.end.x >= start.x && other.end.x <= end.x);
  }
};

struct AnnotatedSlice {
  Slice slice;
  size_t line_number = 0;
  // friend constexpr auto operator<=>(const AnnotatedSlice &lhs,
  //                                   const AnnotatedSlice &rhs) = default;
  friend inline bool operator<(const AnnotatedSlice &l,
                               const AnnotatedSlice &r) {
    if (l.slice != r.slice) {
      return l.slice < r.slice;
    }
    return l.line_number < r.line_number;
  }

  friend inline bool operator==(const AnnotatedSlice &l,
                                const AnnotatedSlice &r) {
    return !(l < r) && !(r < l);
  }
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
        throw std::runtime_error("line number mismatch (slice: " +
                                 std::to_string(slice.line_number) +
                                 "; line: " + std::to_string(line_number) +
                                 ")");
      }
    }
  }

  size_t line_number() const { return _line_number; }
  const std::vector<AnnotatedSlice> &line() const { return _line; }

  void add_slices(const std::vector<AnnotatedSlice> &slices) {
    for (const auto &slice : slices) {
      if (slice.line_number != line_number()) {
        throw std::runtime_error("line number mismatch (slice: " +
                                 std::to_string(slice.line_number) +
                                 "; line: " + std::to_string(line_number()) +
                                 ")");
      }
    }
    _line.insert(_line.end(), slices.begin(), slices.end());
    std::sort(_line.begin(), _line.end());
  }

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
        if (top_slice.slice.touches(bottom_slice.slice)) {
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

  std::string to_string(){
    std::string ret = "Slices{";
    for(const auto &slice_line : slices){
      ret += std::to_string(slice_line.line_number()) + ": ";
      for(const auto &slice : slice_line.line()){
        ret += slice.slice.start.toString() + " -> " + slice.slice.end.toString() + "; ";
      }
    }
    return ret;
  }

  enum class Direction { UP, DOWN };

  bool try_add_image_slices(Slices &image_slices, Direction direction) {
    if (direction == Direction::DOWN) {
      return add_image_slices_down(image_slices);
    } else {
      return add_image_slices_up(image_slices);
    }
  }

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

  void add_slice_line(const SliceLine &slice_line) {
    if (slice_line.line().empty()) {
      return;
    }
    if (slices.empty()) {
      slices.push_back(slice_line);
      return;
    }
    const auto line_number = slice_line.line_number();
    const auto first_line_number = slices.front().line_number();
    const auto last_line_number = slices.back().line_number();
    if (line_number == first_line_number - 1) {
      slices.insert(slices.begin(), slice_line);
    } else if (line_number == last_line_number + 1) {
      slices.push_back(slice_line);
    } else if (first_line_number <= line_number &&
               line_number <= last_line_number) {
      const auto index = line_number - first_line_number;
      slices[index].add_slices(slice_line.line());
    } else {
      throw std::runtime_error("can not add line number " +
                               std::to_string(line_number) + " to slices");
    }
  }

  Direction invert_direction(Direction direction) {
    if (direction == Direction::UP) {
      return Direction::DOWN;
    }
    return Direction::UP;
  }

  SliceLine get_top_line() const {
    if (slices.empty()) {
      return SliceLine{{}, 0};
    }
    return slices.front();
  }

  SliceLine get_bottom_line() const {
    if (slices.empty()) {
      return SliceLine{{}, 0};
    }
    return slices.back();
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
  bool add_image_slices_down(Slices &image_slices) {
    // pre conditions
    // if the image slices are empty nothing to do
    if (image_slices.slices.empty()) {
      return false;
    }
    if (slices.empty()) {
      throw std::runtime_error(
          "slices are empty while trying to add image slices down");
    }
    size_t index = 0;
    bool were_slices_added = false;
    while (index < slices.size()) {
      // get the line one is currently interested in
      auto &current_line = slices[index];
      // get all touching slices in the next line
      auto touching_slices =
          image_slices.extract_touching_slices(current_line, Direction::DOWN);
      if (!touching_slices.empty()) {
        were_slices_added = true;
      }
      if (!touching_slices.empty()) {
        add_slice_line(touching_slices);
      }
      index++;
    }
    return were_slices_added;
  }

  bool add_image_slices_up(Slices &image_slices) {
    // pre conditions
    // if the image slices are empty nothing to do
    if (image_slices.slices.empty()) {
      return false;
    }
    if (slices.empty()) {
      throw std::runtime_error(
          "slices are empty while trying to add image slices down");
    }
    size_t index = 0;
    bool were_slices_added = false;
    while (index < slices.size()) {
      // get the line one is currently interested in
      auto &current_line = slices[slices.size() - 1 - index];
      // get all touching slices in the next line
      auto touching_slices =
          image_slices.extract_touching_slices(current_line, Direction::UP);
      if (!touching_slices.empty()) {
        were_slices_added = true;
      }
      if (!touching_slices.empty()) {
        add_slice_line(touching_slices);
      }
      index++;
    }
    return were_slices_added;
  }

  SliceLine get_next_slice_line(Direction direction, const SliceLine &prev) {
    const auto first_line_number = slices.front().line_number();
    const auto last_line_number = slices.back().line_number();
    if (direction == Direction::DOWN) {
      const auto line_number = prev.line_number() + 1;
      if (line_number > last_line_number) {
        return SliceLine{{}, 0};
      }
      const auto index = line_number - first_line_number;
      if(index >= 0 && index < slices.size()){
        return slices[index];
      }
      return SliceLine{{}, line_number};
    } else {
      const auto line_number = prev.line_number() - 1;
      if (line_number < first_line_number) {
        return SliceLine{{}, 0};
      }
      const auto index = line_number - first_line_number;
      if(index >= 0 && index < slices.size()){
        return slices[index];
      }
      return SliceLine{{}, line_number};
    }
  }

  struct ExtractedSlicesResult {
    std::vector<AnnotatedSlice> touching_slices;
    std::vector<AnnotatedSlice> remaining_slices;
  };

  ExtractedSlicesResult extract_slices(const SliceLine &prev,
                                       const SliceLine &next) {
    std::vector<AnnotatedSlice> touching_slices;
    std::vector<AnnotatedSlice> remaining_slices;
    for (const auto &prev_slice : prev.line()) {
      for (const auto &next_slice : next.line()) {
        if (prev_slice.slice.touches(next_slice.slice)) {
          touching_slices.push_back(next_slice);
        }
      }
    }
    // slices might be added twice by the above algorithm
    // use std::unique to remove duplicates
    const auto last =
        std::unique(touching_slices.begin(), touching_slices.end());
    touching_slices.erase(last, touching_slices.end());
    // bring the touching slices in the right order
    std::sort(touching_slices.begin(), touching_slices.end());

    // figure out the remaining slices
    std::set_difference(next.line().begin(), next.line().end(),
                        touching_slices.begin(), touching_slices.end(),
                        std::back_inserter(remaining_slices));

    return ExtractedSlicesResult{touching_slices, remaining_slices};
  }

  void set_remaining_slices(std::vector<AnnotatedSlice> line, size_t line_number) {
    const auto first_line_number = slices.front().line_number();
    const auto index = line_number - first_line_number;
    if (line.empty()) {
      slices[index] = SliceLine{{}, line_number};
      return;
    }
    slices[index] = SliceLine{line};
  }

  std::vector<AnnotatedSlice> extract_touching_slices(const SliceLine &prev,
                                                      Direction direction) {
    // pre conditions
    if (slices.empty()) {
      return {};
    }

    if (prev.line().empty()) {
      return {};
    }
    auto next = get_next_slice_line(direction, prev);
    if (next.line().empty()) {
      return {};
    }
    auto extracted_slices = extract_slices(prev, next);
    set_remaining_slices(extracted_slices.remaining_slices, next.line_number());
    return extracted_slices.touching_slices;
  }

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

std::vector<std::shared_ptr<Object>> deduce_objects(Slices &slices);

void establishing_shot_single_loop(AllRectangles &ret, const cv::Mat &rgbImage,
                                   const Rectangle &rectangle);

} // namespace od