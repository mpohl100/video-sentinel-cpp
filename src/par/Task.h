#pragma once

#include "par/Work.h"

#include <memory>

namespace par {

class Executor;

class Task {
public:
  Task() = default;
  Task(const Task &) = default;
  Task(Task &&) = default;
  Task &operator=(const Task &) = default;
  Task &operator=(Task &&) = default;
  virtual ~Task() = default;
  Task(std::shared_ptr<Work> work) : _work{work} {}

  void succeed(Task &task);

private:
  std::shared_ptr<Work> get() const { return _work; }
  std::shared_ptr<Work> _work;
  friend bool operator==(const Task &lhs, const Task &rhs);
  friend class Executor;
};

inline bool operator==(const Task &lhs, const Task &rhs) {
  return lhs._work.get() == rhs._work.get();
}


inline void Task::succeed(Task &task) {
#if DO_LOG
  std::cout << "Task::succeed()" << std::endl;
#endif
  _work->add_predecessor(task._work);
}


} // namespace par