#include "coordinated.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace math2d{

// Helper constants and functions
constexpr double PI = 3.141592653589793;
constexpr double to_radians(double degrees) { return degrees * PI / 180.0; }
constexpr double to_degrees(double radians) { return radians * 180.0 / PI; }

// CoordinateSystem Constructor
CoordinateSystem::CoordinateSystem(Point origin, Vector x_axis, Vector y_axis)
    : origin(origin), x_axis(x_axis), y_axis(y_axis) {}

// Convert from CoordinatedPoint to Euclidean Point
Point CoordinateSystem::to_euclidian(const CoordinatedPoint &point) const {
  return Point{
      origin.x + point.x * x_axis.x + point.y * y_axis.x,
      origin.y + point.x * x_axis.y + point.y * y_axis.y};
}

// Convert from Euclidean Point to CoordinatedPoint
CoordinatedPoint CoordinateSystem::from_euclidian(const Point &point) const {
  Vector displacement{point.x - origin.x, point.y - origin.y};

  // Solve for x and y using dot products
  double det = x_axis.x * y_axis.y - x_axis.y * y_axis.x;
  if (std::abs(det) < 1e-9)
    throw std::runtime_error("Axes are linearly dependent or degenerate");

  double xx = (displacement.x * y_axis.y - displacement.y * y_axis.x) / det;
  double yy = (displacement.y * x_axis.x - displacement.x * x_axis.y) / det;

  return CoordinatedPoint(xx, yy, *this);
}

// Rotate the CoordinateSystem around its origin by a given angle
void CoordinateSystem::rotate(const Angle &angle) {
  double cos_a = std::cos(angle.radians());
  double sin_a = std::sin(angle.radians());

  x_axis = Vector{x_axis.x * cos_a - x_axis.y * sin_a,
                  x_axis.x * sin_a + x_axis.y * cos_a};
  y_axis = Vector{y_axis.x * cos_a - y_axis.y * sin_a,
                  y_axis.x * sin_a + y_axis.y * cos_a};
}

// Convert CoordinateSystem to string
std::string CoordinateSystem::to_string() const {
  std::ostringstream oss;
  oss << "Origin: (" << origin.x << ", " << origin.y << ")\n"
      << "X-axis: (" << x_axis.x << ", " << x_axis.y << ")\n"
      << "Y-axis: (" << y_axis.x << ", " << y_axis.y << ")";
  return oss.str();
}

// Convert CoordinatedPoint to a new CoordinateSystem
CoordinatedPoint CoordinatedPoint::convert_to(const CoordinateSystem &new_coordinate_system) const {
  // Convert current point to Euclidean
  Point euclidean_point = coordinate_system.to_euclidian(*this);

  // Convert Euclidean point to the new coordinate system
  return new_coordinate_system.from_euclidian(euclidean_point);
}

// Add a vector to the point
CoordinatedPoint CoordinatedPoint::plus(const Vector &vec) const {
  return CoordinatedPoint{x + vec.x, y + vec.y, coordinate_system};
}

// Convert CoordinatedPoint to string
std::string CoordinatedPoint::toString() const {
  std::ostringstream oss;
  oss << "(" << x << ", " << y << ") in \n" << coordinate_system.to_string();
  return oss.str();
}

// Rotate CoordinatedPoint around another CoordinatedPoint by a given angle
CoordinatedPoint CoordinatedPoint::rotate(const CoordinatedPoint &around_point,
                                           const Angle &angle) const {
  const auto around = around_point.convert_to(coordinate_system);

  double cos_a = std::cos(angle.radians());
  double sin_a = std::sin(angle.radians());

  // Translate the point to the origin of rotation
  double dx = x - around.x;
  double dy = y - around.y;

  // Rotate
  double new_x = dx * cos_a - dy * sin_a;
  double new_y = dx * sin_a + dy * cos_a;

  // Translate back
  return CoordinatedPoint(new_x + around.x, new_y + around.y, coordinate_system);
}

// Distance between two points
number_type CoordinatedPoint::distance_to(const CoordinatedPoint &other_point) const {
  const auto point = other_point.convert_to(coordinate_system);

  double dx = x - point.x;
  double dy = y - point.y;
  return std::sqrt(dx * dx + dy * dy);
}

// Get the Euclidean Point corresponding to this CoordinatedPoint
Point CoordinatedPoint::get_point() const {
  return coordinate_system.to_euclidian(*this);
}

// Operators for comparison
bool operator<(const CoordinatedPoint &l, const CoordinatedPoint &r_cmp) {
  const auto r = r_cmp.convert_to(l.coordinate_system);
  return (l.x < r.x) || (l.x == r.x && l.y < r.y);
}

bool operator==(const CoordinatedPoint &l, const CoordinatedPoint &r_cmp) {
  const auto r = r_cmp.convert_to(l.coordinate_system);  
  constexpr auto epsilon = 1e-9;
  return std::abs(l.x - r.x) < epsilon && std::abs(l.y - r.y) < epsilon;
}

bool operator!=(const CoordinatedPoint &l, const CoordinatedPoint &r) {
  return !(l == r);
}

} // namespace math2d