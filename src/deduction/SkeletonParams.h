#pragma once

namespace deduct {

struct SkeletonParams{
    SkeletonParams() = default;
    SkeletonParams(const SkeletonParams &) = default;
    SkeletonParams(SkeletonParams &&) = default;
    SkeletonParams &operator=(const SkeletonParams &) = default;
    SkeletonParams &operator=(SkeletonParams &&) = default;
    SkeletonParams(int angle_step)
        : angle_step{angle_step} {}
    int angle_step;
};

} // namespace deduct