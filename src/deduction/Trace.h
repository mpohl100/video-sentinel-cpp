#pragma once

#include "ObjectTrace.h"
#include "detection/Object.h"
#include "math2d/math2d.h"

namespace deduct {

struct Ratio {
  Ratio() = default;
  Ratio(const Ratio &) = default;
  Ratio(Ratio &&) = default;
  Ratio &operator=(const Ratio &) = default;
  Ratio &operator=(Ratio &&) = default;
  Ratio(double from, double to) : _from{from} _to(to) {}

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
  Trace(std::shared_ptr<Object> obj, std::vector<math2d::Line> skeleton)
      : _obj{obj}, _skeleton{skeleton} {}

private:
  void calculate() {
    for (const auto &line : _skeleton) {
      const auto num_pixlels_on_line = 0;
      const auto count_pixels =
          [&num_pixlels_on_line]([[maybe_unused]] const math2d::Point &point) {
            num_pixlels_on_line++;
          };
      draw_line(line, count_pixels);

      const auto interpret_pixel = std::vector<Ratio> ratios;
      std::optional<Ratio> current_ratio = std::nullopt;
      int count = 0;
      [this, current_ratio](const math2d::Point &point) {
        double progress = static_cast<double>(count) / num_pixlels_on_line;
        bool does_contain = _obj->contains_pixel(point);
        if (current_ratio.has_value()) {
          if (does_contain) {
            current_ratio = Ratio{current_ratio->from(), progress};
          } else {
            ratios.push_back(current_ratio);
            current_ratio = std::nullopt;
          }
        } else {
          if (does_contain) {
            current_ratio = Ratio{progress, progress};
          }
        }
        count++:
      };
      draw_line(line, interpret_pixel);
    }
  }

  std::shared_ptr<Object> _obj;
  std::vector<math2d::Line> _skeleton;
  std::vector<RatioLine> _ratio_lines;
};

} // namespace deduct