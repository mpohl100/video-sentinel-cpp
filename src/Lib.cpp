#include "Lib.h"

#include <range/v3/all.hpp>

#include <iostream>

namespace lib {

void say_hello(std::string name) {
  std::cout << "Hello, " << name << "!" << '\n';
}

} // namespace lib