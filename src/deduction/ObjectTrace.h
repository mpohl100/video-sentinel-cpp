#pragma once

#include "Trace.h"

#include "detection/Object.h"
#include "math2d/math2d.h"

#include <memory>
#include <vector>

namespace deduct {

struct ObjectTrace {
  ObjectTrace() = default;
  ObjectTrace(const ObjectTrace &) = default;
  ObjectTrace(ObjectTrace &&) = default;
  ObjectTrace &operator=(const ObjectTrace &) = default;
  ObjectTrace &operator=(ObjectTrace &&) = default;
  ObjectTrace(std::shared_ptr<Object> obj) : _obj{obj} { deduce(); }

  Trace get_trace() const { return _trace; }

private:
  std::vector<math2d::Line> get_skeleton(const math2d::Point &center_of_mass,
                                         double radius,
                                         int angle_step) {
    // angle step should be a divisor of 180
    std::vector<math2d::Line> skeleton;
    for (size_t i = 0; i < 180 / angle_step; ++i) {
      const auto angle = math2d::Angle{static_cast<double>(i * angle_step)};
      const auto start = center_of_mass.plus(math2d::Vector{-radius, 0});
      const auto end = center_of_mass.plus(math2d::Vector{radius, 0});
      start.rotate(center_of_mass, angle);
      end.rotate(center_of_mass, angle);
      skeleton.push_back(math2d::Line{start, end});
    }
    return skeleton;
  }

  void deduce() {
    const auto center_of_mass = _obj->get_center_of_mass();
    const auto radius = _obj->get_radius();
    const auto skeleton = get_skeleton(center_of_mass, radius, 30);
    _trace = Trace{_obj, skeleton};
  }
  std::shared_ptr<Object> _obj;
  Trace _trace;
};

} // namespace deduct