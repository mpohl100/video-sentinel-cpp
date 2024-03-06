#pragma once

#include "par/Work.h"
#include "par/Task.h"

#include <type_traits>

namespace par {

template <typename TResult, typename TArgs>
struct ContinuableWorkImpl : public Work {
  ContinuableWorkImpl() = default;
  ContinuableWorkImpl(std::function<TResult(TArgs)> func) : _func{func} {}
  ContinuableWorkImpl(const ContinuableWorkImpl &) = default;
  ContinuableWorkImpl(ContinuableWorkImpl &&) = default;
  ContinuableWorkImpl &operator=(const ContinuableWorkImpl &) = default;
  ContinuableWorkImpl &operator=(ContinuableWorkImpl &&) = default;
  virtual ~ContinuableWorkImpl() = default;

  void call() override { _result = _func(TArgs{}); }

  TResult result() const { return _result; }

private:
  std::function<TResult(TArgs)> _func;
  TResult _result;
};

template <typename TResult, typename TArgs> struct ContinuableWork {
  ContinuableWork() : _impl{std::make_shared<ContinuableWorkImpl<TResult, TArgs>>()} {}
  ContinuableWork(std::function<TResult(TArgs)> func)
      : _impl{std::make_shared<ContinuableWorkImpl<TResult, TArgs>>(func)} {}
  ContinuableWork(const ContinuableWork &) = default;
  ContinuableWork(ContinuableWork &&) = default;
  ContinuableWork &operator=(const ContinuableWork &) = default;
  ContinuableWork &operator=(ContinuableWork &&) = default;
  virtual ~ContinuableWork() = default;
  Task make_task() const {
    return Task{_impl};
  }

private:
  std::shared_ptr<ContinuableWorkImpl<TResult, TArgs>> _impl;
};

} // namespace par