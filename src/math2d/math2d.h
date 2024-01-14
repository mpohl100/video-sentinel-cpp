#pragma once

#include "value_test.h"

#include <cmath>
#include <string>
#include <vector>

namespace math2d{
using number_type = double;
struct Vector;
template <int min_dergrees, int max_degrees> struct RegionedAngle;
using Angle = RegionedAngle<-180, 180>;

struct Point {
  constexpr Point() noexcept = default;
  constexpr Point(const Point &) noexcept = default;
  constexpr Point &operator=(const Point &) noexcept = default;
  constexpr Point(Point &&) noexcept = default;
  constexpr Point &operator=(Point &&) noexcept = default;
  constexpr Point(number_type xx, number_type yy) noexcept : x(xx), y(yy){};

  Point plus(const Vector &vec) const;
  std::string toString() const;
  friend constexpr auto operator<=>(const Point &, const Point &) = default;

  number_type x = 0;
  number_type y = 0;
};
static_assert(value_test::value_test<Point>(),
              "class Point must be value type");

class Line {
public:
  Line() = default;
  Line(const Line &) = default;
  Line &operator=(const Line &) = default;
  Line(Line &&) = default;
  Line &operator=(Line &&) = default;
  Line(Point start, Point end);

  bool intersects(const Line &other) const;

  const Point &start() const;
  const Point &end() const;
  number_type magnitude() const;

  friend constexpr auto operator<=>(const Line &, const Line &) = default;

private:
  Point _start;
  Point _end;
};

struct Vector {
  Vector() = default;
  Vector(const Vector &) = default;
  Vector &operator=(const Vector &) = default;
  Vector(Vector &&) = default;
  Vector &operator=(Vector &&) = default;
  Vector(number_type xx, number_type yy);
  Vector(const Point &start, const Point &end);

  friend constexpr auto operator<=>(const Vector &, const Vector &) = default;

  Vector rotate(const Angle &angle) const;
  Vector scale(double factor) const;
  number_type magnitude() const;

  number_type x = 0.0;
  number_type y = 0.0;
};

template <int min_degrees, int max_degrees> struct RegionedAngle {
  RegionedAngle() = default;
  RegionedAngle(const RegionedAngle &) = default;
  RegionedAngle &operator=(const RegionedAngle &) = default;
  RegionedAngle(RegionedAngle &&) = default;
  RegionedAngle &operator=(RegionedAngle &&) = default;

  RegionedAngle(double degrees) {
    _radians = degrees / 180.0 * M_PI;
    move_to_range();
  }

  RegionedAngle(const Point &p1, const Point &center, const Point &p2) {
    const auto v1 = Vector{center, p1};
    const auto v2 = Vector{center, p2};
    _radians = radians_from_vectors(v1, v2);
    move_to_range();
  }

  RegionedAngle(const Line &line1, const Line &line2) {
    const auto v1 = Vector{line1.start(), line1.end()};
    const auto v2 = Vector{line2.start(), line2.end()};
    _radians = radians_from_vectors(v1, v2);
    move_to_range();
  }

  number_type degrees() const {
    const auto degrees = _radians * 180.0 / M_PI;
    return degrees;
  }
  number_type radians() const { return _radians; }
  number_type min_degrees_value() const { return min_degrees; }
  number_type max_degrees_value() const { return max_degrees; }

private:
  number_type radians_from_vectors(const Vector &v1, const Vector &v2) const {
    const auto dot_product = v1.x * v2.x + v1.y * v2.y;
    const auto magnitude_product = v1.magnitude() * v2.magnitude();
    const auto cross_product = v1.x * v2.y - v1.y * v2.x;
    const auto cos_angle = dot_product / magnitude_product;
    const auto angle = std::acos(cos_angle);
    return cross_product < 0 ? -angle : angle;
  }
  void move_to_range() {
    while (_radians < min_degrees / 180.0 * M_PI) {
      _radians += 2 * M_PI;
    }
    while (_radians > max_degrees / 180.0 * M_PI) {
      _radians -= 2 * M_PI;
    }
  }
  number_type _radians = 0;
};

class Rectangle {
public:
  Rectangle() = default;
  Rectangle(const Rectangle &) = default;
  Rectangle &operator=(const Rectangle &) = default;
  Rectangle(Rectangle &&) = default;
  Rectangle &operator=(Rectangle &&) = default;
  Rectangle(Point tl, Point br);

  friend auto operator<=>(const Rectangle &, const Rectangle &) = default;

  bool intersects(const Rectangle &other) const;

  std::vector<Line> lines() const;
  number_type area() const;
  std::string toString() const;

  std::vector<Line> _lines;
};

Rectangle expand_rectangle(const Rectangle &rect, number_type offset);  

class Circle {
public:
  Circle() = default;
  Circle(const Circle &) = default;
  Circle &operator=(const Circle &) = default;
  Circle(Circle &&) = default;
  Circle &operator=(Circle &&) = default;
  Circle(Point center, number_type radius);

  Rectangle bounding_box() const;
  const Point &center() const;
  number_type radius() const;
  number_type area() const;
  std::string toString() const;
  friend constexpr auto operator<=>(const Circle &, const Circle &) = default;

private:
  Point _center;
  number_type _radius;
};

}