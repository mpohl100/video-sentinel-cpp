#include "Detection.h"
#include "DetectionImpl.h"

#include "opencv2/imgproc.hpp"
#include <iostream>

namespace od {

void detect_directions(cv::Mat &ret, cv::Mat const &bgrImg, const Rectangle& rectangle) {
  detail::detect_edges<detail::DetectionType::Gradient>(ret, bgrImg, rectangle);
}

// cv::Mat detect_edges_gray(cv::Mat const& bgrImg)
//{
//	return detail::detect_edges<detail::DetectionType::Edge>(bgrImg);
// }

void detect_angles(cv::Mat &ret, cv::Mat const &bgrImg, const Rectangle& rectangle) {
  detail::detect_edges<detail::DetectionType::Angle>(ret, bgrImg, rectangle);
}

void smooth_angles(cv::Mat& result, cv::Mat const &angles, int rings, bool onlyRecordAngles,
                      int threshold, const Rectangle& rectangle) {
  if(result.rows != angles.rows || result.cols != angles.cols){
    throw std::runtime_error("uninitialized result mat");
  }
  std::vector<const cv::Vec3b *> rows;
  cv::Vec3b *resultRow = nullptr;
  for (int i = 0; i < 2 * rings + 1; ++i)
    rows.push_back(nullptr);
  for (int i = row_min(rings, rectangle); i < row_max(angles.rows - rings, rectangle); ++i) {
    resultRow = result.ptr<cv::Vec3b>(i);
    size_t index = 0;
    for (int j = i - rings; j < i + rings + 1; ++j)
      rows[index++] = angles.ptr<cv::Vec3b>(j);
    for (int j = col_min(rings, rectangle); j < col_max(angles.cols - rings, rectangle); ++j) {
      double sumAngle = 0;
      double sumLen = 0;

      for (int k = 0; k < int(rows.size()); ++k)
        for (int l = j - rings; l < j + rings + 1; ++l) {

          double len = double(rows[k][l][0]);
          if (len > 0) {
            sumLen += len;
            int angle = rows[k][l][1];
            if (angle > 0)
              sumAngle += len * angle;
            else
              sumAngle -= len * rows[k][l][2];
          }
        }

      int nb = 2 * rings + 1;
      nb *= nb; // squared
      if (sumLen / nb < threshold) {
        resultRow[j][0] = 255;
        resultRow[j][1] = 255;
        resultRow[j][2] = 255;
      } else {
        double angle = sumAngle / sumLen;
        double len = sumLen / nb;
        resultRow[j][0] = int(len);
        if (angle > 0) {
          if (!onlyRecordAngles) {
            resultRow[j][1] = int(angle);
            resultRow[j][2] = 0;
          } else {
            resultRow[j][0] = int(angle * 256.0 / 180.0);
            resultRow[j][1] = 0;
            resultRow[j][2] = 0;
          }
        } else {
          if (!onlyRecordAngles) {
            resultRow[j][1] = 0;
            resultRow[j][2] = int(-angle);
          } else {
            resultRow[j][0] = 0;
            int angleInt = int(-angle * 256.0 / 180.0);
            resultRow[j][1] = angleInt;
            resultRow[j][2] = 0;
          }
        }
      }
    }
  }
}

} // namespace od