#pragma once

#include "ComparisonParams.h"
#include "Draw.h"
#include "detection/Object.h"
#include "math2d/math2d.h"

namespace deduct {

struct Ratio {
  Ratio() = default;
  Ratio(const Ratio &) = default;
  Ratio(Ratio &&) = default;
  Ratio &operator=(const Ratio &) = default;
  Ratio &operator=(Ratio &&) = default;
  Ratio(double from, double to) : _from{from}, _to(to) {}

  double from() const { return _from; }
  double to() const { return _to; }

private:
  double _from;
  double _to;
};

struct RatioLine {
  RatioLine() = default;
  RatioLine(const RatioLine &) = default;
  RatioLine(RatioLine &&) = default;
  RatioLine &operator=(const RatioLine &) = default;
  RatioLine &operator=(RatioLine &&) = default;
  RatioLine(math2d::Line line, std::vector<Ratio> ratios)
      : _line{line}, _ratios{ratios} {}

  math2d::Line get_line() const { return _line; }
  const std::vector<Ratio> &get_ratios() const { return _ratios; }

private:
  math2d::Line _line;
  std::vector<Ratio> _ratios;
};

struct Trace {
  Trace() = default;
  Trace(const Trace &) = default;
  Trace(Trace &&) = default;
  Trace &operator=(const Trace &) = default;
  Trace &operator=(Trace &&) = default;
  Trace(od::Object obj, std::vector<math2d::Line> skeleton,
        SkeletonParams skeleton_params)
      : _obj{obj}, _skeleton{skeleton}, _skeleton_params{skeleton_params} {
    calculate();
  }

  std::vector<RatioLine> get_ratios() const { return _ratio_lines; }

  bool compare(const Trace &other,
               const ComparisonParams &comparison_params) const {
    // pre conditions
    if (_obj == other._obj) {
      return true;
    }

    if (_skeleton_params != other._skeleton_params) {
      return false;
    }

    if (_skeleton.size() != other._skeleton.size()) {
      return false;
    }

    for (size_t i = 0; i < _ratio_lines.size(); ++i) {
      bool matches = compare_ratio_lines(_ratio_lines, other._ratio_lines, i,
                                         comparison_params);
      if (matches) {
        return true;
      }
    }
    return false;
  }

private:
  void calculate() {
    _ratio_lines.clear();
    _ratio_lines.reserve(_skeleton.size());
    for (const auto &line : _skeleton) {
      const auto pixels = draw_line(line);
      const auto num_pixels_on_line = pixels.size();

      std::vector<Ratio> ratios;
      std::optional<Ratio> current_ratio = std::nullopt;
      int count = 0;
      const auto interpret_pixel = [this, &current_ratio, &count,
                                    num_pixels_on_line,
                                    &ratios](const math2d::Point &point) {
        double progress = static_cast<double>(count) / num_pixels_on_line;
        bool does_contain = _obj.contains_point(point);
        if (current_ratio.has_value()) {
          if (does_contain) {
            current_ratio = Ratio{current_ratio->from(), progress};
          } else {
            ratios.push_back(*current_ratio);
            current_ratio = std::nullopt;
          }
        } else {
          if (does_contain) {
            current_ratio = Ratio{progress, progress};
          }
        }
        count++;
      };
      for (const auto &pixel : pixels) {
        interpret_pixel(pixel);
      }
      _ratio_lines.push_back(RatioLine{line, ratios});
    }
  }

  bool compare_ratio_lines(const std::vector<RatioLine> &lhs,
                           const std::vector<RatioLine> &rhs, size_t index,
                           const ComparisonParams &comparison_params) const {
    const auto get_secondary_index = [&rhs, index](size_t i) {
      return (i + index) % rhs.size();
    };

    for (size_t i = 0; i < lhs.size(); ++i) {
      bool matches = compare_ratio_line(lhs[i], rhs[get_secondary_index(i)],
                                        comparison_params);
      if (!matches) {
        return false;
      }
    }
    return true;
  }

  bool compare_ratio_line(const RatioLine &lhs, const RatioLine &rhs,
                          const ComparisonParams &comparison_params) const {
    if(lhs.get_ratios().empty() || rhs.get_ratios().empty()) {
      if(lhs.get_ratios().empty() && rhs.get_ratios().empty()){
        return true;
      }
      return false;
    }
    if (comparison_params.only_outer_form) {
      const auto left_ratio =
          Ratio{lhs.get_ratios().front().from(), lhs.get_ratios().back().to()};
      const auto right_ratio =
          Ratio{rhs.get_ratios().front().from(), rhs.get_ratios().back().to()};
      return compare_ratio(left_ratio, right_ratio,
                           comparison_params.tolerance);
    }

    if (lhs.get_ratios().size() != rhs.get_ratios().size()) {
      return false;
    }

    for (size_t i = 0; i < lhs.get_ratios().size(); ++i) {
      if (!compare_ratio(lhs.get_ratios()[i], rhs.get_ratios()[i],
                         comparison_params.tolerance)) {
        return false;
      }
    }
    return true;
  }

  bool compare_ratio(const Ratio &lhs, const Ratio &rhs,
                     double tolerance) const {
    return std::abs(lhs.from() - rhs.from()) < tolerance &&
           std::abs(lhs.to() - rhs.to()) < tolerance;
  }

  od::Object _obj;
  std::vector<math2d::Line> _skeleton;
  std::vector<RatioLine> _ratio_lines;
  SkeletonParams _skeleton_params;
};

} // namespace deduct