#pragma once

#include "math2d/math2d.h"

namespace od{

struct Rectangle {
  Rectangle() = default;
  Rectangle(const Rectangle &) = default;
  Rectangle &operator=(const Rectangle &) = default;
  Rectangle(Rectangle &&) = default;
  Rectangle &operator=(Rectangle &&) = default;
  Rectangle(const math2d::Rectangle &rectangle);
  Rectangle(int xx, int yy, int w, int h) : x{xx}, y{yy}, width{w}, height{h} {}

  math2d::Rectangle to_math2d_rectangle() const;

  int x = 0;
  int y = 0;
  int width = 1;
  int height = 1;
};

inline Rectangle::Rectangle(const math2d::Rectangle &rectangle)
    : x{static_cast<int>(rectangle.lines()[0].start().x)},
      y{static_cast<int>(rectangle.lines()[0].start().y)},
      width{static_cast<int>(rectangle.lines()[1].end().x -
                             rectangle.lines()[0].start().x)},
      height{static_cast<int>(rectangle.lines()[1].end().y -
                              rectangle.lines()[0].start().y)} {}

inline math2d::Rectangle Rectangle::to_math2d_rectangle() const {
  return math2d::Rectangle{
      math2d::Point{static_cast<math2d::number_type>(x),
                    static_cast<math2d::number_type>(y)},
      math2d::Point{static_cast<math2d::number_type>(x + width),
                    static_cast<math2d::number_type>(y + height)}};
}

inline int col_min(int start, const Rectangle& rectangle)
{
    return std::max(start, rectangle.x);
}

inline int col_max(int end, const Rectangle& rectangle)
{
    return std::min(end, rectangle.x + rectangle.width);
}

inline int row_min(int start, const Rectangle& rectangle)
{
    return std::max(start, rectangle.y);
}

inline int row_max(int end, const Rectangle& rectangle)
{
    return std::min(end, rectangle.y + rectangle.height);
}



}