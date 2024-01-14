#pragma once

#include "Rectangle.h"
#include "opencv2/core/mat.hpp"
#include <cmath>

namespace od {

void detect_directions(cv::Mat &ret, cv::Mat const &bgrImg, const od::Rectangle& rectangle);
// cv::Mat detect_edges_gray(cv::Mat const& bgrImg);
void detect_angles(cv::Mat &ret, cv::Mat const &bgrImg, const od::Rectangle& rectangle);

void smooth_angles(cv::Mat &result, cv::Mat const &angles, int rings, bool onlyRecordAngles, int threshold, const od::Rectangle& rectangle);

} // namespace od