#pragma once

#include "math2d/math2d.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>

namespace deduct {

inline std::vector<math2d::Point> draw_line(const math2d::Line &line) {
  std::vector<math2d::Point> pixels;
  const auto draw_pixel = [&pixels](int x, int y, [[maybe_unused]] int color) {
    pixels.push_back(math2d::Point{static_cast<math2d::number_type>(x), static_cast<math2d::number_type>(y)});
  };
  auto start = line.start();
  auto end = line.end();
  bool inverted = false;
  if (end.x < start.x) {
    std::swap(start, end);
    inverted = true;
  }
  constexpr bool dolog = false;
  int dX = end.x - start.x;
  int dY = end.y - start.y;
  math2d::Point current_point = start;
  // draw start and end point
  // draw_pixel(start.x, start.y, 1);
  // draw_pixel(end.x, end.y, 1);
  if (dX == 0) {
    if (dY >= 0) {
      for (size_t i = 0; i <= static_cast<size_t>(dY); ++i) {
        draw_pixel(current_point.x, current_point.y, 1);
        current_point.y++;
      }
    } else {
      for (size_t i = 0; i <= static_cast<size_t>(-dY); ++i) {
        draw_pixel(current_point.x, current_point.y, 1);
        current_point.y--;
      }
    }
    return pixels;
  }
  if (dY == 0) {
    if (dX >= 0) {
      if constexpr (dolog)
        std::cout << "x positive\n";
      for (size_t i = 0; i <= static_cast<size_t>(dX); ++i) {
        draw_pixel(current_point.x, current_point.y, 1);
        current_point.x++;
      }
    } else {
      for (size_t i = 0; i <= static_cast<size_t>(-dX); ++i) {
        draw_pixel(current_point.x, current_point.y, 1);
        current_point.x--;
      }
    }
    return pixels;
  }

  const auto gradient = double(dY) / double(dX);
  if constexpr (dolog) {
    std::cout << "gradient = " << gradient << '\n';
    std::cout << "dX=" << dX << "; dY=" << dY << '\n';
  }
  enum class Direction {
    X,
    YUp,
    YDown,
  };
  const auto move_coord = [](int &d, math2d::Point &point, int &went,
                             Direction direction) {
    switch (direction) {
    case Direction::X: {
      d--;
      point.x++;
      went++;
      break;
    }
    case Direction::YUp: {
      d--;
      point.y++;
      went++;
      break;
    }
    case Direction::YDown: {
      d++;
      point.y--;
      went--;
      break;
    }
    }
  };
  const auto go_x = [&](double went_x, double went_y) {
    const auto deduce_current_gradient = [=]() {
      if (went_x == 0) {
        if (went_y > 0) {
          return std::numeric_limits<double>::max();
        } else if (went_y == 0) {
          return 0.0;
        } else {
          return -std::numeric_limits<double>::max();
        }
      }
      return went_y / went_x;
    };
    if constexpr (dolog) {
      std::cout << "went_x = " << went_x << "; went_y = " << went_y << "\n";
    }
    const auto current_gradient = deduce_current_gradient();
    if constexpr (dolog) {
      std::cout << "current gradient = " << current_gradient
                << "; gradient = " << gradient << '\n';
    }
    if (gradient >= 0) {
      if (current_gradient > gradient) {
        if constexpr (dolog) {
          std::cout << "go x\n";
        }
        return Direction::X;
      }
      if constexpr (dolog) {
        std::cout << "go y up\n";
      }
      return Direction::YUp;
    } else {
      if (current_gradient < gradient) {
        if constexpr (dolog) {
          std::cout << "go x\n";
        }
        return Direction::X;
      }
      if constexpr (dolog) {
        std::cout << "go y down\n";
      }
      return Direction::YDown;
    }
  };
  int went_x = 0;
  int went_y = 0;
  while (true) {
    if (dX == 0 && dY == 0) {
      if (current_point != end) {
        throw std::runtime_error("end point not hit in draw_line.");
      }
      draw_pixel(current_point.x, current_point.y, 1);
      break;
    }
    if constexpr (dolog) {
      std::cout << "setting point to 1: x=" << current_point.x
                << "; y=" << current_point.y << "; dX=" << dX << "; dY=" << dY
                << '\n';
    }
    draw_pixel(current_point.x, current_point.y, 1);
    const auto direction = go_x(went_x, went_y);
    switch (direction) {
    case Direction::X:
      move_coord(dX, current_point, went_x, direction);
      break;
    case Direction::YUp:
    case Direction::YDown:
      move_coord(dY, current_point, went_y, direction);
      break;
    }
  }
  if (inverted) {
    std::reverse(pixels.begin(), pixels.end());
  }
  return pixels;
}

} // namespace deduct