#pragma once

#include "par/Work.h"
#include "par/Calculation.h"
#include "par/Task.h"


#include <memory>

namespace par {

class Task;

class FlowImpl : public Work {
public:
  FlowImpl() : Work{}, _work{} {}
  FlowImpl(FlowImpl &&) = default;
  FlowImpl &operator=(FlowImpl &&) = default;
  virtual ~FlowImpl() = default;

  void add(const Calculation &work) { _work.push_back(work.get()); }
  void add(std::shared_ptr<FlowImpl> work) { _work.push_back(work); }

  void call() override {
#if DO_LOG
    std::cout << "FlowImpl::call()" << std::endl;
#endif
    for (auto &work : _work) {
      work->call();
    }
  }

private:
  std::vector<std::shared_ptr<Work>> _work;
};

class Flow {
public:
  Flow() : _impl{std::make_shared<FlowImpl>()} {}
  Flow(Flow &&) = default;
  Flow &operator=(Flow &&) = default;
  virtual ~Flow() = default;

  void add(const Calculation &work) { _impl->add(work); }
  void add(const Flow &work) { _impl->add(work._impl); }

  Task make_task() { return Task{_impl}; }

private:
  std::shared_ptr<FlowImpl> _impl;
};


} // namespace par