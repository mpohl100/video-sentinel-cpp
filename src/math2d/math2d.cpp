#include "math2d.h"

#include <cmath>
#include <string>

namespace math2d{
    
Vector::Vector(double xx, double yy) : x(xx), y(yy) {}

Vector::Vector(const Point &start, const Point &end)
    : x(end.x - start.x), y(end.y - start.y) {}

Vector Vector::rotate(const Angle &angle) const {
  const auto radians = angle.radians();
  const auto cos_angle = std::cos(radians);
  const auto sin_angle = std::sin(radians);
  const auto dx = 0;
  const auto dy = 0;
  const auto x_rotated = ((x - dx) * cos_angle) - ((dy - y) * sin_angle) + dx;
  const auto y_rotated = dy - ((dy - y) * cos_angle) + ((x - dx) * sin_angle);
  return Vector{x_rotated, y_rotated};
}

Vector Vector::scale(double factor) const {
  return Vector{x * factor, y * factor};
}

double Vector::magnitude() const {
  return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
}

Point Point::plus(const Vector &vec) const {
  return Point{x + vec.x, y + vec.y};
}

std::string Point::toString() const {
  return "Point{x: " + std::to_string(x) + "; y: " + std::to_string(y) + "}";
}

Line::Line(Point start, Point end) : _start(start), _end(end) {}

bool Line::intersects(const Line &other) const
{
    const auto &p = _start;
    const auto &q = other._start;
    const auto &r = Vector(_start, _end);
    const auto &s = Vector(other._start, other._end);
    const auto r_cross_s = r.x * s.y - r.y * s.x;
    const auto q_minus_p = Vector(q, p);
    const auto q_minus_p_cross_r = q_minus_p.x * r.y - q_minus_p.y * r.x;
    const auto q_minus_p_cross_s = q_minus_p.x * s.y - q_minus_p.y * s.x;
    if (r_cross_s == 0 && q_minus_p_cross_r == 0) {
        // lines are collinear
        const auto t0 = q_minus_p.x / r.x;
        const auto t1 = (q_minus_p.x + s.x) / r.x;
        const auto t2 = q_minus_p.y / r.y;
        const auto t3 = (q_minus_p.y + s.y) / r.y;
        const auto t_min = std::min({t0, t1, t2, t3});
        const auto t_max = std::max({t0, t1, t2, t3});
        return t_min <= 1 && t_max >= 0;
    }
    if (r_cross_s == 0 && q_minus_p_cross_r != 0) {
        // lines are parallel and non-intersecting
        return false;
    }
    const auto t = q_minus_p_cross_s / r_cross_s;
    const auto u = q_minus_p_cross_r / r_cross_s;
    return t >= 0 && t <= 1 && u >= 0 && u <= 1;
}

const Point &Line::start() const { return _start; }

const Point &Line::end() const { return _end; }

double Line::magnitude() const {
  return std::sqrt(std::pow(_end.x - _start.x, 2) +
                   std::pow(_end.y - _start.y, 2));
}

Rectangle::Rectangle(Point tl, Point br) : _lines() {
  _lines.emplace_back(tl, Point(br.x, tl.y));
  _lines.emplace_back(Point(br.x, tl.y), br);
  _lines.emplace_back(br, Point(tl.x, br.y));
  _lines.emplace_back(Point(tl.x, br.y), tl);
}

bool Rectangle::intersects(const Rectangle &other) const{
    const auto &other_lines = other.lines();
    for (const auto &line : _lines) {
        for (const auto &other_line : other_lines) {
            if (line.intersects(other_line)) {
                return true;
            }
        }
    }
    return false;
}

std::vector<Line> Rectangle::lines() const { return _lines; }

double Rectangle::area() const {
  return _lines[0].magnitude() * _lines[1].magnitude();
}

std::string Rectangle::toString() const
{
    return "Rectangle{top_left: " + _lines[0].start().toString() +
           "; bottom_right: " + _lines[1].end().toString() + "}";
}


Rectangle expand_rectangle(const Rectangle &rect, number_type offset)
{
    const auto &top_left = rect.lines()[0].start();
    const auto &bottom_right = rect.lines()[1].end();
    return Rectangle(Point(top_left.x - offset, top_left.y - offset),
                     Point(bottom_right.x + offset, bottom_right.y + offset));

}


Circle::Circle(Point center, number_type radius)
    : _center(center), _radius(radius) {}

Rectangle Circle::bounding_box() const
{
    return Rectangle(Point(_center.x - _radius, _center.y - _radius),
                     Point(_center.x + _radius, _center.y + _radius));
}

const Point &Circle::center() const { return _center; }

number_type Circle::radius() const { return _radius; }

double Circle::area() const { return M_PI * std::pow(_radius, 2); }

std::string Circle::toString() const {
  return "Circle{center: " + _center.toString() +
         "; radius: " + std::to_string(_radius) + "}";
}

}