#pragma once

#include "ComparisonParams.h"
#include "Draw.h"
#include "Skeleton.h"
#include "detection/Object.h"
#include "math2d/math2d.h"

#include <algorithm>

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
  RatioLine(Skeleton::Area area, std::vector<Ratio> ratios)
      : _area{area}, _ratios{ratios} {}

  Skeleton::Area get_area() const { return _area; }
  const std::vector<Ratio> &get_ratios() const { return _ratios; }

private:
  Skeleton::Area _area;
  std::vector<Ratio> _ratios;
};

struct Trace {
  Trace() = default;
  Trace(const Trace &) = default;
  Trace(Trace &&) = default;
  Trace &operator=(const Trace &) = default;
  Trace &operator=(Trace &&) = default;
  Trace(od::Object obj, Skeleton skeleton,
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

    if (_skeleton.areas.size() != other._skeleton.areas.size()) {
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

  bool compare_integral(const Trace &other,
                        const ComparisonParams &comparison_params) const {
    // pre conditions
    if (_obj == other._obj) {
      return true;
    }

    if (_skeleton_params != other._skeleton_params) {
      return false;
    }

    if (_skeleton.areas.size() != other._skeleton.areas.size()) {
      return false;
    }

    const auto this_max_offset = deduce_max_offset(_ratio_lines);
    const auto other_max_offset = deduce_max_offset(other._ratio_lines);

    const auto max_offset = std::max(this_max_offset, other_max_offset);

    bool within_tolerance = true;
    for (size_t offset = 0; offset < max_offset; ++offset) {
      const auto this_integral = calculate_integral(_ratio_lines, 0);
      const auto other_integral = calculate_integral(other._ratio_lines, 0);
      // overload the meaning of tolerance to be the difference between the
      // integrals
      within_tolerance =
          within_tolerance && std::abs(this_integral - other_integral) <
                                  comparison_params.tolerance;

      if (comparison_params.only_outer_form) {
        break;
      }
    }
    return within_tolerance;
  }

private:
  void calculate() {
    _ratio_lines.clear();
    _ratio_lines.reserve(_skeleton.areas.size());
    for (const auto &area : _skeleton.areas) {
      std::vector<Ratio> ratios;
      std::optional<Ratio> current_ratio = std::nullopt;
      int count = 0;
      const auto interpret_pixel = [this, &current_ratio, &count,
                                    &ratios](const math2d::CoordinatedPoint &point) {
        double progress = static_cast<double>(count) / _skeleton_params.nb_parts_of_object;
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
      const auto start = math2d::CoordinatedPoint{0, 0, area.coordinate_system}.plus(math2d::Vector{-area.radius, 0});
      const auto delta = math2d::Vector{2.0 * area.radius / _skeleton_params.nb_parts_of_object, 0};
      for(size_t i = 0; i < _skeleton_params.nb_parts_of_object; ++i) {
        const auto point = start.plus(delta.scale(i));
        interpret_pixel(point);
      }
      _ratio_lines.push_back(RatioLine{area, ratios});
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
    if (lhs.get_ratios().empty() || rhs.get_ratios().empty()) {
      if (lhs.get_ratios().empty() && rhs.get_ratios().empty()) {
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

  double calculate_integral(const std::vector<RatioLine> &ratio_lines,
                            size_t offset) const {
    const auto values = deduce_relevant_values(ratio_lines, offset);
    // the sum of the individual values is the integral
    return std::accumulate(values.begin(), values.end(), 0.0);
  }

  std::vector<double>
  deduce_relevant_values(const std::vector<RatioLine> &ratio_lines,
                         size_t offset) const {
    double angle_step = 180.0 / _ratio_lines.size();
    std::vector<double> values;
    values.reserve(2 * ratio_lines.size());
    const auto get_element_from_front =
        [offset](const std::vector<Ratio> &ratios) {
          int index = offset / 2;
          if (ratios.empty()) {
            return 0.5;
          }
          if (ratios.size() <= index) {
            return 0.5;
          }
          if (offset % 2 == 0) {
            return ratios[index].from();
          }
          return ratios[index].to();
        };

    const auto get_element_from_back =
        [offset](const std::vector<Ratio> &ratios) {
          int index = offset / 2;
          if (ratios.empty()) {
            return 0.5;
          }
          if (ratios.size() <= index) {
            return 0.5;
          }
          if (offset % 2 == 0) {
            return ratios[ratios.size() - 1 - index].to();
          }
          return ratios[ratios.size() - 1 - index].from();
        };

    for (size_t i = 0; i < 2 * ratio_lines.size(); ++i) {
      double current_angle = i * angle_step;
      if (current_angle < 180) {
        const auto &ratios = ratio_lines[i % ratio_lines.size()].get_ratios();
        if (ratios.empty()) {
          values.push_back(0.0);
          continue;
        }
        double value = get_element_from_back(ratios) - 0.5;
        values.push_back(value);
      } else {
        const auto &ratios = ratio_lines[i % ratio_lines.size()].get_ratios();
        if (ratios.empty()) {
          values.push_back(0.0);
          continue;
        }
        double value = 0.5 - get_element_from_front(ratios);
        values.push_back(value);
      }
    }
    return values;
  }

  size_t deduce_max_offset(const std::vector<RatioLine> &ratios) const {
    size_t max_offset = 0;
    for (const auto &ratio_line : ratios) {
      max_offset = std::max(max_offset, 2 * ratio_line.get_ratios().size());
    }
    return max_offset;
  }

  od::Object _obj;
  Skeleton _skeleton;
  std::vector<RatioLine> _ratio_lines;
  SkeletonParams _skeleton_params;
};

} // namespace deduct