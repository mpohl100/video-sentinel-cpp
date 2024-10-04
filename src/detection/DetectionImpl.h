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
  float sqrt2 = 1.0 / std::sqrt(2.0);
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
inline void detect_edges(cv::Mat &ret, cv::Mat const &bgrImg,
                         const od::Rectangle &rectangle) {
  if (ret.rows != bgrImg.rows || ret.cols != bgrImg.cols) {
    throw std::runtime_error("uninitialized ret mat");
  }

  auto roiRect =
      cv::Rect(rectangle.x - 1, rectangle.y - 1, rectangle.width + 3, rectangle.height + 3);

  roiRect = roiRect & cv::Rect(0, 0, bgrImg.cols, bgrImg.rows);

  cv::Mat roi = bgrImg(roiRect);
  // Convert the BGR image to Grayscale in the region of interest
  cv::Mat grayImage;
  cv::cvtColor(roi, grayImage, cv::COLOR_RGB2GRAY);

  const auto get_checked = [grayImage](int i, int j){
    if(i < 0 || i >= grayImage.rows || j < 0 || j >= grayImage.cols){
      throw std::runtime_error("Index out of bounds i:" + std::to_string(i) + "; j:" + std::to_string(j) + "; rows:" + std::to_string(grayImage.rows) + "; cols:" + std::to_string(grayImage.cols));
    }
    return static_cast<int>(grayImage.at<uchar>(i, j));
  };

  cv::Vec3b *retCenter;
  int y = 1;
  for (int i = roiRect.y + 1;
       i < roiRect.y + roiRect.height - 1; ++i) {
    if constexpr (detectionType == DetectionType::Gradient ||
                  detectionType == DetectionType::Angle) {
      retCenter = ret.ptr<cv::Vec3b>(i);
    }
    int x = 1;
    for (int j = roiRect.x + 1;
         j < roiRect.x + roiRect.width - 1; ++j) {
      int degrees = 0;
      auto ret_val = gradient<detectionType, 0>(
          get_checked(y - 1, x - 1),
          get_checked(y - 1, x),
          get_checked(y - 1, x + 1),
          get_checked(y, x - 1),
          get_checked(y, x),
          get_checked(y, x + 1),
          get_checked(y + 1, x - 1),
          get_checked(y + 1, x),
          get_checked(y + 1, x + 1));
      int grad_c = 0;
      if constexpr (detectionType == DetectionType::Edge) {
        grad_c = ret_val;
      } else {
        grad_c = ret_val.first;
        degrees = ret_val.second;
      }
      auto val = float(grad_c);

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
      x++;
    }
    y++;
  }
}
}; // namespace detail