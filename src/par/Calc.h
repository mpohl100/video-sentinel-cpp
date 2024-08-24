#pragma once

#include "par/Work.h"
#include "par/Task.h"
#include "par/Executor.h"

#include <cassert>
#include <type_traits>

namespace par {

template<typename> class CalcImpl;

template <typename TResult, typename TArg>
struct CalcImpl<TResult(TArg)> : public Work {
  CalcImpl() = default;
  CalcImpl(std::function<TResult(TArg)> func) : _func{func} {}
  CalcImpl(const CalcImpl &) = default;
  CalcImpl(CalcImpl &&) = default;
  CalcImpl &operator=(const CalcImpl &) = default;
  CalcImpl &operator=(CalcImpl &&) = default;
  virtual ~CalcImpl() = default;

  void call() override { _result = _func(_arg); }

  TResult result() const { return _result; }
  TArg arg() const { return _arg; }
  void set_arg(TArg arg) { _arg = arg; }
private:
  std::function<TResult(TArg)> _func;
  TResult _result;
  TArg _arg;
};

template <typename TResult>
struct CalcImpl<TResult()> : public Work {
  CalcImpl() = default;
  CalcImpl(std::function<TResult()> func) : _func{func} {}
  CalcImpl(const CalcImpl &) = default;
  CalcImpl(CalcImpl &&) = default;
  CalcImpl &operator=(const CalcImpl &) = default;
  CalcImpl &operator=(CalcImpl &&) = default;
  virtual ~CalcImpl() = default;

  void call() override { _result = _func(); }

  TResult result() const { return _result; }
private:
  std::function<TResult()> _func;
  TResult _result;
};

template <typename TArg>
struct CalcImpl<void(TArg)> : public Work {
  CalcImpl() = default;
  CalcImpl(std::function<void(TArg)> func) : _func{func} {}
  CalcImpl(const CalcImpl &) = default;
  CalcImpl(CalcImpl &&) = default;
  CalcImpl &operator=(const CalcImpl &) = default;
  CalcImpl &operator=(CalcImpl &&) = default;
  virtual ~CalcImpl() = default;

  void call() override { _func(_arg); }

  TArg arg() const { return _arg; }
  void set_arg(TArg arg) { _arg = arg; }
private:
  std::function<void(TArg)> _func;
  TArg _arg;
};

template<>
struct CalcImpl<void()> : public Work {
  CalcImpl() = default;
  CalcImpl(std::function<void()> func) : _func{func} {}
  CalcImpl(const CalcImpl &) = default;
  CalcImpl(CalcImpl &&) = default;
  CalcImpl &operator=(const CalcImpl &) = default;
  CalcImpl &operator=(CalcImpl &&) = default;
  virtual ~CalcImpl() = default;

  void call() override { _func(); }

private:
  std::function<void()> _func;
};

template <typename> struct Calc;

template<>
struct Calc<void()> {
  Calc() : _impl{std::make_shared<CalcImpl<void()>>()} {}
  Calc(std::function<void()> func)
      : _impl{std::make_shared<CalcImpl<void()>>(func)} {}
  Calc(const Calc &) = default;
  Calc(Calc &&) = default;
  Calc &operator=(const Calc &) = default;
  Calc &operator=(Calc &&) = default;
  virtual ~Calc() = default;
  Task make_task() const {
    return Task{_impl};
  }

  bool is_finished() const {
    return _impl->is_finished();
  }

private:
  std::shared_ptr<CalcImpl<void()>> _impl;
};

template <typename TResult, typename TArg> 
struct Calc<TResult(TArg)> {
  Calc() : _impl{std::make_shared<CalcImpl<TResult(TArg)>>()} {}
  Calc(std::function<TResult(TArg)> func)
      : _impl{std::make_shared<CalcImpl<TResult(TArg)>>(func)} {}
  Calc(const Calc &) = default;
  Calc(Calc &&) = default;
  Calc &operator=(const Calc &) = default;
  Calc &operator=(Calc &&) = default;
  virtual ~Calc() = default;
  Task make_task() const {
    return Task{_impl};
  }

  Calc<void()> then(Executor& executor, std::function<void(TResult)> continuation)
  {
    auto this_calc = *this;
    auto func = [this_calc, continuation]() {
      assert(this_calc.is_finished());
      continuation(this_calc.result());
    };
    auto continuation_calc = Calc<void()>(func);
    auto continuation_task = continuation_calc.make_task();
    continuation_task.succeed(this->make_task());
    executor.run(continuation_task);
    return continuation_calc;
  }

  template<typename ThenResult>
  Calc<ThenResult()> then(Executor& executor, std::function<ThenResult(TResult)> continuation)
  {
    auto this_calc = *this;
    auto func = [this_calc, continuation]() {
      assert(this_calc.is_finished());
      return continuation(this_calc.result());
    };
    auto continuation_calc = Calc<ThenResult()>(func);
    auto continuation_task = continuation_calc.make_task();
    continuation_task.succeed(this->make_task());
    executor.run(continuation_task);
    return continuation_calc;
  }

  TResult result() const {
    return _impl->result();
  }

  bool is_finished() const {
    return _impl->is_finished();
  }

private:
  std::shared_ptr<CalcImpl<TResult(TArg)>> _impl;
};

template <typename TResult> 
struct Calc<TResult()> {
  Calc() : _impl{std::make_shared<CalcImpl<TResult()>>()} {}
  Calc(std::function<TResult()> func)
      : _impl{std::make_shared<CalcImpl<TResult()>>(func)} {}
  Calc(const Calc &) = default;
  Calc(Calc &&) = default;
  Calc &operator=(const Calc &) = default;
  Calc &operator=(Calc &&) = default;
  virtual ~Calc() = default;
  Task make_task() const {
    return Task{_impl};
  }

  Calc<void()> then(Executor& executor, std::function<void(TResult)> continuation)
  {
    auto this_calc = *this;
    auto func = [this_calc, continuation]() {
      assert(this_calc.is_finished());
      continuation(this_calc.result());
    };
    auto continuation_calc = Calc<void()>(func);
    auto continuation_task = continuation_calc.make_task();
    auto this_task = this->make_task();
    continuation_task.succeed(this_task);
    executor.run(continuation_task);
    return continuation_calc;
  }

  template<typename ThenResult>
  Calc<ThenResult()> then(Executor& executor, std::function<ThenResult(TResult)> continuation)
  {
    auto this_calc = *this;
    auto func = [this_calc, continuation]() {
      assert(this_calc.is_finished());
      return continuation(this_calc.result());
    };
    auto continuation_calc = Calc<ThenResult()>(func);
    auto continuation_task = continuation_calc.make_task();
    auto this_task = make_task();
    continuation_task.succeed(this_task);
    executor.run(continuation_task);
    return continuation_calc;
  }

  TResult result() const {
    return _impl->result();
  }

  bool is_finished() const {
    return _impl->is_finished();
  }


private:
  std::shared_ptr<CalcImpl<TResult()>> _impl;
};


template <typename TArg> 
struct Calc<void(TArg)> {
  Calc() : _impl{std::make_shared<CalcImpl<void(TArg)>>()} {}
  Calc(std::function<void(TArg)> func)
      : _impl{std::make_shared<CalcImpl<void(TArg)>>(func)} {}
  Calc(const Calc &) = default;
  Calc(Calc &&) = default;
  Calc &operator=(const Calc &) = default;
  Calc &operator=(Calc &&) = default;
  virtual ~Calc() = default;
  Task make_task() const {
    return Task{_impl};
  }

  bool is_finished() const {
    return _impl->is_finished();
  }


private:
  std::shared_ptr<CalcImpl<void(TArg)>> _impl;
};

} // namespace par