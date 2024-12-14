#pragma once

#include "SkeletonParams.h"
#include "Trace.h"

#include "detection/Object.h"
#include "math2d/math2d.h"
#include "math2d/coordinated.h"

#include <memory>
#include <vector>

namespace deduct {

struct ObjectTrace {
  ObjectTrace() = default;
  ObjectTrace(const ObjectTrace &) = default;
  ObjectTrace(ObjectTrace &&) = default;
  ObjectTrace &operator=(const ObjectTrace &) = default;
  ObjectTrace &operator=(ObjectTrace &&) = default;
  ObjectTrace(od::Object obj, SkeletonParams skeleton_params) 
  : _obj{obj}, _skeleton_params{skeleton_params} { deduce(); } 

  Trace get_trace() const { return _trace; }


private:
  std::vector<math2d::Line> get_skeleton(const math2d::CoordinatedPoint &center_of_mass,
                                         double radius,
                                         SkeletonParams skeleton_params) {
    // angle step should be a divisor of 180
    std::vector<math2d::Line> skeleton;
    for (size_t i = 0; i < 180 / skeleton_params.angle_step; ++i) {
      const auto angle = math2d::Angle{static_cast<double>(i * skeleton_params.angle_step)};
      const auto start = center_of_mass.plus(math2d::Vector{-radius, 0});
      const auto end = center_of_mass.plus(math2d::Vector{radius, 0});
      start.rotate(center_of_mass, angle);
      end.rotate(center_of_mass, angle);
      skeleton.push_back(math2d::Line{start, end});
    }
    return skeleton;
  }

  void deduce() {
    const auto center_of_mass = _obj.get_center_of_mass();
    const auto point_of_max_distance = _obj.get_point_of_max_distance_to(center_of_mass);
    const auto radius = center_of_mass.distance_to(point_of_max_distance);
    const auto skeleton = get_skeleton(center_of_mass, radius, _skeleton_params);
    _trace = {Trace{_obj, skeleton, _skeleton_params}};
  }

  od::Object _obj;
  Trace _trace;
  SkeletonParams _skeleton_params;
};

} // namespace deduct