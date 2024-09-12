#pragma once

#include "detection/Object.h"
#include "math2d/math2d.h"
#include "ObjectTrace.h"

namespace deduct {

struct Ratio{
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

struct RatioLine{
    RatioLine() = default;
    RatioLine(const RatioLine &) = default;
    RatioLine(RatioLine &&) = default;
    RatioLine &operator=(const RatioLine &) = default;
    RatioLine &operator=(RatioLine &&) = default;
    RatioLine(math2d::Line line, std::vector<Ratio> ratios) : _line{line}, _ratios{ratios} {}

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
    std::shared_ptr<Object> _obj;
    std::vector<math2d::Line> _skeleton;
    std::vector<RatioLine> _ratio_lines;
};

} // namespace deduct