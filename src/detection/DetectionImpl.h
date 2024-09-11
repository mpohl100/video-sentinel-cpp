#pragma once

#include "Rectangle.h"

#include "opencv2/imgproc.hpp"

#include <iostream>
#include <stdexcept>

namespace detail {

enum class DetectionType {
  Edge,
  Gradient,
  Angle,
};

static int logCounter = 0;
/**
 * returns the gradient over a pixel
 * first calculate the x and y gradients over the pixel cc
 * then return the sqrt(grad_x**2 + grad_y**2)
 */
template <DetectionType detectionType, int threshold>
inline auto gradient(int tl, int tc, int tr, int cl, int cc, int cr, int bl,
                     int bc, int br) {
  int grad = 0;
  float grad_tl_br = br - tl;
  float grad_cl_cr = cr - cl;
  float grad_bl_tr = tr - bl;
  float grad_bc_tc = tc - bc;
  float sqrt2 = std::sqrt(2.0);
  float grad_x = grad_cl_cr + grad_tl_br * sqrt2 + grad_bl_tr * sqrt2;
  float grad_y = -grad_bc_tc + grad_tl_br * sqrt2 - grad_bl_tr * sqrt2;
  float grad_total = std::sqrt(grad_x * grad_x + grad_y * grad_y);
  grad = int(grad_total);
  if constexpr (false) {
    std::cout << "gradient of:\n";
    std::cout << tl << " " << tc << " " << tl << '\n';
    std::cout << cl << " " << cc << " " << cl << '\n';
    std::cout << bl << " " << bc << " " << bl << '\n';
    std::cout << "grad_x = " << grad_x << "; grad_y = " << grad_y << "\n";
    std::cout << "grad = " << grad << '\n';
  }
  if constexpr (detectionType == DetectionType::Edge)
    return grad;
  else {
    if (grad <= threshold)
      return std::pair<int, int>{0, 0};
    float cosAlpha = grad_x / grad_total;
    float radians = std::acos(cosAlpha);
    float degrees = radians * (180.0 / 3.1415926);
    if (grad_y < 0)
      degrees *= -1.0;
    int degrees_ret = int(degrees);
    if constexpr (false) {
      if (logCounter++ < 1000)
        std::cout << grad << " " << degrees_ret << "\n";
    }
    return std::pair<int, int>{grad, degrees_ret};
  }
}
/**
 * returns a gray scale Mat, takes an BGR / RGB Mat
 * iterate over all pixels and calculate the color gradient over it
 * the gray scale return matrix represents the ratio of the biggest color
 * channel in relation to the sum of all color channels a white pixel means a
 * super high descent and a black pixel means a planar surface
 */
template <DetectionType detectionType>
inline void detect_edges(cv::Mat& ret, cv::Mat const &bgrImg, const od::Rectangle& rectangle){
  if(ret.rows != bgrImg.rows || ret.cols != bgrImg.cols){
    throw std::runtime_error("uninitialized ret mat");
  }
  cv::Vec3b *retCenter;
  const cv::Vec3b *imgUpper;
  const cv::Vec3b *imgCenter;
  const cv::Vec3b *imgLower;
  for (int i = od::row_min(1, rectangle); i < od::row_max(bgrImg.rows - 1, rectangle); ++i) {
    if constexpr (detectionType == DetectionType::Gradient ||
                  detectionType == DetectionType::Angle)
      retCenter = ret.ptr<cv::Vec3b>(i);
    imgUpper = bgrImg.ptr<cv::Vec3b>(i - 1);
    imgCenter = bgrImg.ptr<cv::Vec3b>(i);
    imgLower = bgrImg.ptr<cv::Vec3b>(i + 1);
    for (int j = od::col_min(1, rectangle); j < od::col_max(bgrImg.cols - 1, rectangle); ++j) {
      int max = 0;
      int sum = 0;
      int degrees = 0;
      for (int color = 0; color <= 2; color++) {
        auto ret = gradient<detectionType, 0>(
            imgUpper[j - 1][color], imgUpper[j][color], imgUpper[j + 1][color],
            imgCenter[j - 1][color], imgCenter[j][color],
            imgCenter[j + 1][color], imgLower[j - 1][color], imgLower[j][color],
            imgLower[j + 1][color]);
        int grad_c = 0;
        if constexpr (detectionType == DetectionType::Edge) {
          grad_c = ret;
        } else {
          grad_c = ret.first;
          degrees = ret.second;
        }
        max = grad_c > max ? grad_c : max;
        sum += grad_c;
      }
      float val = float(max) / float(sum);
      val = std::sqrt(val);
      val *= max;

      if constexpr (detectionType == DetectionType::Edge) {
        ret.at<uchar>(i, j) = int(val);
      } else if (detectionType == DetectionType::Gradient) {
        retCenter[j][0] = int(val);
        if (degrees >= 0) {
          retCenter[j][1] = degrees;
          retCenter[j][2] = 0;
        } else {
          retCenter[j][1] = 0;
          retCenter[j][2] = -degrees;
        }
      } else {
        retCenter[j][0] = 255;
        retCenter[j][1] = 255;
        retCenter[j][2] = 255;
        if (val > 0) {
          if (degrees >= 0) {
            retCenter[j][0] = int(float(degrees) * 256.0 / 180.0);
            retCenter[j][1] = 0;
            retCenter[j][2] = 0;
          } else {
            retCenter[j][0] = 0;
            retCenter[j][1] = int(float(-degrees) * 256.0 / 180.0);
            retCenter[j][2] = 0;
          }
        }
      }
    }
  }
}
}; // namespace detail