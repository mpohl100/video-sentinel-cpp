#include <catch2/catch_all.hpp>

#include "par/ContinuableWork.h"

#include <iostream>
#include <vector>

namespace {

TEST_CASE("ContinuableTask", "[continuable_task]") {

  SECTION("ContinuableTaskThenExecutesAllFunctionsOfTheChain") {
    auto return_function = std::function<int(int)>([](int i) -> int { return i++; });
    auto work = par::ContinuableWork<int, int>{return_function};
  }

}

} // namespace