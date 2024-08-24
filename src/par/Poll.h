#pragma once

#include "Calc.h"
#include "Executor.h"
#include "Task.h"

#include <chrono>

namespace par {

template <typename TResultWait, typename TResultContinue> struct Poll {
  Poll() = default;
  Poll(const Poll &) = default;
  Poll(Poll &&) = default;
  Poll &operator=(const Poll &) = default;
  Poll &operator=(Poll &&) = default;

  Poll(Executor &executor, Calc<TResultWait()> calc_to_wait_for,
       std::function<TResultContinue(TResultWait)> func_to_continue)
      : _executor{executor}, _calc_to_wait_for{calc_to_wait_for},
        _func_to_continue{func_to_continue} {
    _executor.run(_calc_to_wait_for.make_task());
  }

  Task make_task() const {
    return Calc<void()>([this] { poll(); }).make_task();
  }

  void poll() const {
    if (!_executor.wait_for(_calc_to_wait_for.make_task(),
                            std::chrono::microseconds{0})) {
      auto this_task = make_task();
      _executor.run_in(this_task, _timeout);
      return;
    }
    auto calc_to_continue = Calc<TResultContinue()>(
        [this]() { return _func_to_continue(_calc_to_wait_for.result()); });
    _executor.run(calc_to_continue.make_task());
  }

private:
  Executor &_executor;
  Calc<TResultWait()> _calc_to_wait_for;
  std::function<TResultContinue(TResultWait)> _func_to_continue;
  std::chrono::microseconds _timeout{50};
};

} // namespace par