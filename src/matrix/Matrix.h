#pragma once

#include <vector>

namespace matrix {

template <class T> class Matrix {
public:
  Matrix(const size_t width, const size_t height) {
    _width = width;
    _height = height;
    _data = std::vector<T>(width * height, T{});
  }

  Matrix() = default;
  Matrix(const Matrix &) = default;
  Matrix(Matrix &&) = default;
  Matrix &operator=(const Matrix &) = default;
  Matrix &operator=(Matrix &&) = default;

  T &get(size_t x, size_t y) { return _data[x + y * _width]; }
  const T &get(size_t x, size_t y) const { return _data[x + y * _width]; }

  size_t width() const { return _width; }

  size_t height() const { return _height; }

private:
  size_t _width = 1;
  size_t _height = 1;
  std::vector<T> _data = {T{}};
};

} // namespace matrix