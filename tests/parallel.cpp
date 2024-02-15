#include <catch2/catch_all.hpp>

#include "par/parallel.h"

#include <iostream>
#include <vector>

namespace {

struct GotCalledVoidFunction {
private:
  bool _got_called = false;
public:
  bool got_called() const { return _got_called; }
  void operator()() { got_called = true; }
};

} // namespace

TEST_CASE("Parallel", "[parallel]") {

  SECTION("ExecutorExecutesScheduledCalculation") {
    auto got_called_function = GotCalledVoidFunction{}; 
    auto calculation = par::Calculation{got_called_function};
    auto task = calculation.make_task();
    auto executor = par::Executor{4};
    
    CHECK_FALSE(got_called_function.got_called());
    executor.run(task);
    executor.wait_for(task);
    
    CHECK(got_called_function.got_called());
    CHECK(executor.does_not_know(task))
  }
  SECTION("ExecutorExecutesScheduledFlow"){
    std::vector<GotCalledVoidFunction> got_called_functions{4};
    auto flow = par::Flow{};
    for(auto& got_called_function : got_called_functions){
      flow.add(par::Calculation{got_called_function});
    }
    auto task = flow.make_task();
    auto executor = par::Executor{4};

    CHECK_FALSE(got_called_functions[0].got_called());
    CHECK_FALSE(got_called_functions[1].got_called());
    CHECK_FALSE(got_called_functions[2].got_called());
    CHECK_FALSE(got_called_functions[3].got_called());

    executor.run(task);
    executor.wait_for(task);

    CHECK(got_called_functions[0].got_called());
    CHECK(got_called_functions[1].got_called());
    CHECK(got_called_functions[2].got_called());
    CHECK(got_called_functions[3].got_called());
    CHECK(executor.does_not_know(task));
  }
  SECTION("ExecutorExecutesScheduledFlowWithNestedFlows"){
    std::vector<GotCalledVoidFunction> got_called_functions{4};
    auto flow = par::Flow{};
    flow.add(par::Calculation{got_called_function[0]});
    auto nested_flow = par::Flow{};
    for(size_t i = 1; i < got_called_functions.size(); ++i){
      nested_flow.add(par::Calculation{got_called_functions[i]});
    }
    flow.add(nested_flow);
    auto task = flow.make_task();
    auto executor = par::Executor{4};

    CHECK_FALSE(got_called_functions[0].got_called());
    CHECK_FALSE(got_called_functions[1].got_called());
    CHECK_FALSE(got_called_functions[2].got_called());
    CHECK_FALSE(got_called_functions[3].got_called());

    executor.run(task);
    executor.wait_for(task);

    CHECK(got_called_functions[0].got_called());
    CHECK(got_called_functions[1].got_called());
    CHECK(got_called_functions[2].got_called());
    CHECK(got_called_functions[3].got_called());
    CHECK(executor.does_not_know(task));
  }
  SECTION("NotWaitedForPredecesseorTasksAreRemoved"){
    std::vector<GotCalledVoidFunction> got_called_functions{4};
    std::vector<par::Task> tasks;
    for(auto& got_called_function : got_called_functions){
      auto calculation = par::Calculation{got_called_function};
      auto task = calculation.make_task();
      tasks.push_back(task);
    }
    for(size_t i = 1; i < tasks.size(); ++i){
      tasks[i].succeeds(tasks[i-1]);
    }
    auto executor = par::Executor{4};
    CHECK_FALSE(got_called_functions[0].got_called());
    CHECK_FALSE(got_called_functions[1].got_called());
    CHECK_FALSE(got_called_functions[2].got_called());
    CHECK_FALSE(got_called_functions[3].got_called());

    executor.run(tasks.back());
    executor.wait_for(tasks.back());

    CHECK(got_called_functions[0].got_called());
    CHECK(got_called_functions[1].got_called());
    CHECK(got_called_functions[2].got_called());
    CHECK(got_called_functions[3].got_called());
    for(auto task : tasks){
      CHECK(executor.does_not_know(task));
    }
  }
}