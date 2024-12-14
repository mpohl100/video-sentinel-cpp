#include "coordinated.h"

namespace math2d {

CoordinateSystem::CoordinateSystem(Point origin, Vector x_axis, Vector y_axis)
    : origin{origin}, x_axis{x_axis}, y_axis{y_axis} {}

Point CoordinateSystem::to_euclidian(const CoordinatedPoint &point) const
{
  const auto x = point.x * x_axis.x + point.y * y_axis.x + origin.x;
  const auto y = point.x * x_axis.y + point.y * y_axis.y + origin.y;
  return Point{x, y};
}

CoordinatedPoint CoordinateSystem::from_euclidian(const Point &point) const
{
  const auto x = (point.x - origin.x) * x_axis.x + (point.y - origin.y) * x_axis.y;
  const auto y = (point.x - origin.x) * y_axis.x + (point.y - origin.y) * y_axis.y;
  return CoordinatedPoint{x, y, *this};
}

void CoordinateSystem::rotate(const Angle &angle)
{
  const auto x_axis_rotated = x_axis.rotate(angle);
  const auto y_axis_rotated = y_axis.rotate(angle);
  x_axis = x_axis_rotated;
  y_axis = y_axis_rotated;
}

/// @brief This function adds a vector to a point. It assumes that the vector is
/// in the same coordinate system as the point.
/// @param vec the vector to add to the point
/// @return the coordinate point that is the result of adding the vector to the
/// original point in this
CoordinatedPoint CoordinatedPoint::plus(const Vector &vec) const {
  const auto x = this->x + vec.x;
  const auto y = this->y + vec.y;
  return CoordinatedPoint{x, y, this->coordinate_system};
}

std::string CoordinatedPoint::toString() const {
  return std::string("CoordinatedPoint(") + std::to_string(x) + ", " +
         std::to_string(y) + ")";
}

CoordinatedPoint CoordinatedPoint::rotate(const CoordinatedPoint &around,
                                          const Angle &angle) const {
  const auto this_point_euclidian = coordinate_system.to_euclidian(*this);
  const auto around_point_euclidian =
      around.coordinate_system.to_euclidian(around);
  const auto rotated_point_euclidian =
      this_point_euclidian.rotate(around_point_euclidian, angle);
  return coordinate_system.from_euclidian(rotated_point_euclidian);
}

bool operator<(const CoordinatedPoint &l, const CoordinatedPoint &r);
bool operator==(const CoordinatedPoint &l, const CoordinatedPoint &r);
bool operator!=(const CoordinatedPoint &l, const CoordinatedPoint &r);

} // namespace math2d