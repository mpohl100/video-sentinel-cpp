#pragma once

#include "math2d.h"

#include <string>

namespace math2d {

struct CoordinatedPoint;

struct CoordinateSystem {
  CoordinateSystem() = default;
  CoordinateSystem(const CoordinateSystem &) = default;
  CoordinateSystem &operator=(const CoordinateSystem &) = default;
  CoordinateSystem(CoordinateSystem &&) = default;
  CoordinateSystem &operator=(CoordinateSystem &&) = default;

  CoordinateSystem(Point origin, Vector x_axis, Vector y_axis);

  Point to_euclidian(const CoordinatedPoint &point) const;
  CoordinatedPoint from_euclidian(const Point &point) const;

  void rotate(const Angle &angle);

  std::string to_string() const;

  Point origin;  // origin of the coordinate system in standard euclidean
                 // coordinates
  Vector x_axis; // x-axis of the coordinate system in standard euclidean
                 // coordinates
  Vector y_axis; // y-axis of the coordinate system in standard euclidean
                 // coordinates
};

struct CoordinatedPoint {
  constexpr CoordinatedPoint() noexcept = default;
  constexpr CoordinatedPoint(const CoordinatedPoint &) noexcept = default;
  constexpr CoordinatedPoint &
  operator=(const CoordinatedPoint &) noexcept = default;
  constexpr CoordinatedPoint(CoordinatedPoint &&) noexcept = default;
  constexpr CoordinatedPoint &operator=(CoordinatedPoint &&) noexcept = default;
  constexpr CoordinatedPoint(number_type xx, number_type yy,
                             CoordinateSystem coordinate_system) noexcept
      : x(xx), y(yy), coordinate_system(coordinate_system){};

  CoordinatedPoint convert_to(const CoordinateSystem &new_coordinate_system) const;
  CoordinatedPoint plus(const Vector &vec) const;
  std::string toString() const;
  CoordinatedPoint rotate(const CoordinatedPoint &around,
                          const Angle &angle) const;

  number_type distance_to(const CoordinatedPoint & point) const;

  Point get_point() const;
  // friend constexpr auto operator<=>(const CoordinatedPoint &, const
  // CoordinatedPoint &) = default;
  friend bool operator<(const CoordinatedPoint &l, const CoordinatedPoint &r);
  friend bool operator==(const CoordinatedPoint &l, const CoordinatedPoint &r);
  friend bool operator!=(const CoordinatedPoint &l, const CoordinatedPoint &r);

  number_type x = 0;
  number_type y = 0;
  CoordinateSystem coordinate_system;
};

#if 0

struct CoordinatedVector {
  CoordinatedVector() = default;
  CoordinatedVector(const CoordinatedVector &) = default;
  CoordinatedVector &operator=(const CoordinatedVector &) = default;
  CoordinatedVector(CoordinatedVector &&) = default;
  CoordinatedVector &operator=(CoordinatedVector &&) = default;
  CoordinatedVector(number_type xx, number_type yy,
                    CoordinateSystem coordinate_system);
  CoordinatedVector(const Point &start, const Point &end);

  // friend constexpr auto operator<=>(const CoordinatedVector &, const
  // CoordinatedVector &) = default;

  CoordinatedVector rotate(const Angle &angle) const;
  CoordinatedVector scale(double factor) const;
  number_type magnitude() const;

  number_type x = 0.0;
  number_type y = 0.0;
  CoordinateSystem coordinate_system;
};

#endif

} // namespace math2d