#pragma once

#include "math2d/math2d.h"

#include <stdexcept>
#include <string>

namespace od{

struct Rectangle {
  Rectangle() = default;
  Rectangle(const Rectangle &) = default;
  Rectangle &operator=(const Rectangle &) = default;
  Rectangle(Rectangle &&) = default;
  Rectangle &operator=(Rectangle &&) = default;
  Rectangle(const math2d::Rectangle &rectangle);
  Rectangle(int xx, int yy, int w, int h) : x{xx}, y{yy}, width{w}, height{h} {}
  Rectangle(math2d::Point start, math2d::Point end)
      : x{static_cast<int>(start.x)},
        y{static_cast<int>(start.y)},
        width{static_cast<int>(end.x - start.x)},
        height{static_cast<int>(end.y - start.y)} {}

  math2d::Rectangle to_math2d_rectangle() const;

  bool contains(const Rectangle &other) const {
    return x <= other.x && y <= other.y &&
           x + width >= other.x + other.width &&
           y + height >= other.y + other.height;
  }

  void merge_right(const Rectangle &other) {
    if((x + width) != other.x){
      throw std::runtime_error("Cannot merge right because of x! this: " + to_string() + "; other: " + other.to_string());
    }
    if(y != other.y){
      throw std::runtime_error("Cannot merge right because of y! this: " + to_string() + "; other: " + other.to_string());
    }
    if(height != other.height){
      throw std::runtime_error("Cannot merge right because of height! this: " + to_string() + "; other: " + other.to_string());
    }
    width += other.width;
  }

  void merge_down(const Rectangle &rectangle){
    if(x != rectangle.x){
      throw std::runtime_error("Cannot merge down because of x! this: " + to_string() + "; other: " + rectangle.to_string());
    }
    if((y + height) != rectangle.y){
      throw std::runtime_error("Cannot merge down because of y! this: " + to_string() + "; other: " + rectangle.to_string());
    }
    if(width != rectangle.width){
      throw std::runtime_error("Cannot merge down because of width! this: " + to_string() + "; other: " + rectangle.to_string());
    }
    height += rectangle.height;
  }

  std::string to_string() const{
    return "Rectangle{x: " + std::to_string(x) + "; y: " + std::to_string(y) + "; width: " + std::to_string(width) + "; height: " + std::to_string(height) + "}";
  }

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