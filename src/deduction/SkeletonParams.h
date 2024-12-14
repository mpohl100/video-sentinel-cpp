#pragma once

namespace deduct {

struct SkeletonParams {
  SkeletonParams() = default;
  SkeletonParams(const SkeletonParams &) = default;
  SkeletonParams(SkeletonParams &&) = default;
  SkeletonParams &operator=(const SkeletonParams &) = default;
  SkeletonParams &operator=(SkeletonParams &&) = default;
  SkeletonParams(int angle_step, int nb_parts_of_object)
      : angle_step{angle_step}, nb_parts_of_object{nb_parts_of_object} {}
  int angle_step;
  int nb_parts_of_object = 30;
};

inline bool operator==(const SkeletonParams &lhs, const SkeletonParams &rhs) {
  return lhs.angle_step == rhs.angle_step &&
         lhs.nb_parts_of_object == rhs.nb_parts_of_object;
}

inline bool operator!=(const SkeletonParams &lhs, const SkeletonParams &rhs) {
  return !(lhs == rhs);
}

} // namespace deduct