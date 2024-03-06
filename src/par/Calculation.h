#pragma once

#include "par/Work.h"
#include "par/Task.h"

#include <functional>
#include <memory>

namespace par {

class FlowImpl;
class Task;

class CalculationImpl : public Work {
public:
  CalculationImpl(std::function<void()> func) : Work{}, _func{func} {}
  CalculationImpl() = default;
  CalculationImpl(const CalculationImpl &) = default;
  CalculationImpl(CalculationImpl &&) = default;
  CalculationImpl &operator=(const CalculationImpl &) = default;
  CalculationImpl &operator=(CalculationImpl &&) = default;
  virtual ~CalculationImpl() = default;

  void call() override {
#if DO_LOG
    std::cout << "CalculationImpl::call()" << std::endl;
#endif
    _func();
  }

  std::function<void()> _func;
};

class Calculation {
public:
  Calculation(std::function<void()> func)
      : _impl{std::make_shared<CalculationImpl>(func)} {}
  Calculation() = default;
  Calculation(const Calculation &) = default;
  Calculation(Calculation &&) = default;
  Calculation &operator=(const Calculation &) = default;
  Calculation &operator=(Calculation &&) = default;
  virtual ~Calculation() = default;

  void call() {
#if DO_LOG
    std::cout << "Calculation::call()" << std::endl;
#endif
    _impl->call();
  }

  Task make_task() { return Task{_impl}; }

private:
  std::shared_ptr<CalculationImpl> get() const { return _impl; }
  std::shared_ptr<CalculationImpl> _impl;
  friend class FlowImpl;
};

}