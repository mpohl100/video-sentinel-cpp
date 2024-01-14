#pragma once

#include <type_traits>
#include <utility>

namespace value_test {

template <class T> consteval bool value_test() {
  static_assert(std::is_nothrow_default_constructible_v<T>,
                "type must be nothrow default constructible");
  static_assert(std::is_nothrow_copy_constructible_v<T>,
                "type must be nothrow copy constructible");
  static_assert(std::is_nothrow_move_constructible_v<T>,
                "type must be nothrow move constructible");
  static_assert(std::is_nothrow_copy_assignable_v<T>,
                "type must be nothrow copy assignable");
  static_assert(std::is_nothrow_move_assignable_v<T>,
                "type must be nothrow move assignable");

  // test constexpr default constructor
  [[maybe_unused]] constexpr auto t = T{};
  // test constexpr copy constructor
  [[maybe_unused]] constexpr auto copy = t;
  // test constexpr move constructor
  [[maybe_unused]] constexpr auto move = std::move(t);

  return true;
}

} // namespace value_test