#pragma once

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
  Trace(od::Object obj, std::vector<math2d::Line> skeleton)
      : _obj{obj}, _skeleton{skeleton} {
    calculate();
  }

  std::vector<RatioLine> get_ratios() const { return _ratio_lines; }

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
        bool does_contain = _obj->contains_point(point);
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

  od::Object _obj;
  std::vector<math2d::Line> _skeleton;
  std::vector<RatioLine> _ratio_lines;
};

} // namespace deduct