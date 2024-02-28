#include <catch2/catch_all.hpp>

#include "par/parallel.h"

#include <iostream>
#include <vector>

namespace {

TEST_CASE("Parallel", "[parallel]") {

  SECTION("ExecutorExecutesScheduledCalculation") {
    int i = 0;
    auto void_function = [&i](){ i++; };
    auto calculation = par::Calculation{void_function};
    auto task = calculation.make_task();
    auto executor = par::Executor{4};
    
    CHECK(i == 0);

    executor.run(task);
    executor.wait_for(task);
    
    CHECK(i == 1);
    CHECK(executor.does_not_know(task));
  }
  SECTION("ExecutorExecutesScheduledFlow"){
    int i = 0;
    auto void_function = [&i](){ i++; };
    auto flow = par::Flow{};
    for(size_t j = 0; j < 4; ++j){
      flow.add(par::Calculation{void_function});
    }
    auto task = flow.make_task();
    auto executor = par::Executor{4};

    CHECK(i == 0);

    executor.run(task);
    executor.wait_for(task);

    CHECK(i == 4);
    CHECK(executor.does_not_know(task));
  }
  SECTION("ExecutorExecutesScheduledFlowWithNestedFlows"){
    int i = 0;
    auto void_function = [&i](){ i++; };
    auto flow = par::Flow{};
    flow.add(par::Calculation{void_function});
    auto nested_flow = par::Flow{};
    for(size_t j = 1; j < 4; ++j){
      nested_flow.add(par::Calculation{void_function});
    }
    flow.add(nested_flow);
    auto task = flow.make_task();
    auto executor = par::Executor{4};

    CHECK(i == 0);

    executor.run(task);
    executor.wait_for(task);

    CHECK(i == 4);
    CHECK(executor.does_not_know(task));
  }
  SECTION("NotWaitedForPredecesseorTasksAreRemoved"){
    int i = 0;
    auto void_function = [&i](){ i++; };
    std::vector<par::Task> tasks;
    for(size_t j = 0; j < 4; ++j){
      auto calculation = par::Calculation{void_function};
      auto task = calculation.make_task();
      tasks.push_back(task);
    }
    for(size_t i = 1; i < tasks.size(); ++i){
      tasks[i].succeed(tasks[i-1]);
    }
    auto executor = par::Executor{4};

    CHECK(i == 0);

    for(auto task : tasks){
      executor.run(task);
    }
    executor.wait_for(tasks.back());

    CHECK(i == 4);
    for(auto task : tasks){
      CHECK(executor.does_not_know(task));
    }
  }
}

} // namespace