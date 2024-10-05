#pragma once

#include "par/Calculation.h"
#include "par/Task.h"

#include <vector>

namespace par {

struct TaskGraph {
  TaskGraph() = default;
  TaskGraph(const TaskGraph &) = default;
  TaskGraph(TaskGraph &&) = default;
  TaskGraph &operator=(const TaskGraph &) = default;
  TaskGraph &operator=(TaskGraph &&) = default;

  void add_task(Task task) {
    _tasks.front().succeed(task);
    _tasks.push_back(task);
  }

  std::vector<Task> get_tasks() const { return _tasks; }

private:
  Task create_dummy_finish_task() {
    auto calc = Calculation{[]() -> void {}};
    return calc.make_task();
  }

  std::vector<Task> _tasks = {create_dummy_finish_task()};
};

}