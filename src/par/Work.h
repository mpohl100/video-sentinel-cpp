#pragma once

#include <algorithm>
#include <memory>
#include <vector>

namespace par{

class Work {
public:
  Work() = default;
  Work(const Work &) = default;
  Work(Work &&) = default;
  Work &operator=(const Work &) = default;
  Work &operator=(Work &&) = default;
  virtual ~Work() = default;

  virtual void call() = 0;
  void add_predecessor(const std::shared_ptr<Work> &work) {
    _predecessors.push_back(work);
  }
  bool can_be_started() const {
    return std::all_of(
        _predecessors.cbegin(), _predecessors.cend(),
        [](const std::shared_ptr<Work> &work) { return work->is_finished(); });
  }
  bool is_finished() const { return _finished; }
  void set_finished() { _finished = true; }
  const std::vector<std::shared_ptr<Work>> &get_predecessors() const {
    return _predecessors;
  }

private:
  std::vector<std::shared_ptr<Work>> _predecessors;
  bool _finished = false;
};

} // namespace par