#pragma once

#include "par/Calculation.h"
#include "par/ContinuableWork.h"
#include "par/Flow.h"
#include "par/Task.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#define DO_LOG 0

namespace par {

class Work;
class FlowImpl;
class Executor;

class Executor {
public:
  Executor() = default;
  Executor(Executor &&) = default;
  Executor &operator=(Executor &&) = default;
  ~Executor() {
#if DO_LOG
    std::cout << "Executor::~Executor()" << std::endl;
#endif
    cancel_all();
    _main_thread.join();
  }

  Executor(int num_threads) : _mutex{std::make_shared<std::mutex>()} {
    async_init(num_threads);
  }

  void run(Task task) {
#if DO_LOG
    std::cout << "Executor::run()" << std::endl;
#endif
    std::unique_lock<std::mutex> lock(*_mutex);
    _queued_tasks.push_back(task);
  }

  bool erase_work_from_finished(Task work, bool do_lock = true) {
    std::optional<std::unique_lock<std::mutex>> lock;
    if(do_lock){
      lock = std::unique_lock<std::mutex>{*_mutex};
    }
    auto it = std::find(_finished_tasks.begin(), _finished_tasks.end(), work);
    if (it == _finished_tasks.end()) {
      return false;
    }
    _finished_tasks.erase(
        std::remove(_finished_tasks.begin(), _finished_tasks.end(), work),
        _finished_tasks.end());
    for (auto predecessor : work._work->get_predecessors()) {
      erase_work_from_finished(Task{predecessor}, /*do_lock=*/false);
    }
    return true;
  }

  void wait_for(Task work) {
#if DO_LOG
    std::cout << "Executor::wait_for()" << std::endl;
#endif
    while (true) {
#if DO_LOG
      std::cout << "Executor::wait_for() checking if finished" << std::endl;
#endif
      if (erase_work_from_finished(work)) {
        break;
      }
#if DO_LOG
      std::cout << "Executor::wait_for() sleep 1ms" << std::endl;
#endif
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
#if DO_LOG
    std::cout << "Executor::wait_for() return" << std::endl;
#endif
  }

  bool does_not_know(Task work) {
    std::unique_lock<std::mutex> lock(*_mutex);
    bool not_in_queued_tasks =
        std::find(_queued_tasks.begin(), _queued_tasks.end(), work) ==
        _queued_tasks.end();
    bool not_in_scheduled_tasks =
        std::find(_scheduled_tasks.begin(), _scheduled_tasks.end(), work) ==
        _scheduled_tasks.end();
    bool not_in_started_tasks =
        std::find(_started_tasks.begin(), _started_tasks.end(), work) ==
        _started_tasks.end();
    bool not_in_finished_tasks =
        std::find(_finished_tasks.begin(), _finished_tasks.end(), work) ==
        _finished_tasks.end();
    return not_in_queued_tasks && not_in_scheduled_tasks &&
           not_in_started_tasks && not_in_finished_tasks;
  }

private:
  void async_init(size_t num_threads) {
#if DO_LOG
    std::cout << "Executor::async_init()" << std::endl;
#endif
    _main_thread = std::thread{[this, num_threads]() {
      for (size_t i = 0; i < num_threads; ++i) {
        _worker_threads.emplace_back([this]() { execute_worker_thread(); });
      }
      for (auto &thread : _worker_threads) {
        thread.join();
      }
    }};
  }

  void execute_worker_thread() {
    for (;;) {
      bool shall_I_cancel = false;
      {
        std::unique_lock<std::mutex> lock(*_mutex);
        shall_I_cancel = _cancelled;
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() shall I cancel?"
                  << std::endl;
#endif
      }
      if (shall_I_cancel) {
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() do cancel?"
                  << std::endl;
#endif
        break;
      }
      schedule_task();
      auto work = pop_task();
      if (!work) {
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() sleep 1ms" << std::endl;
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      } else {
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() do_work" << std::endl;
#endif
        work->get()->call();
        work->get()->set_finished();
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() finished_work"
                  << std::endl;
#endif
        std::unique_lock<std::mutex> lock(*_mutex);
        _started_tasks.erase(
            std::remove(_started_tasks.begin(), _started_tasks.end(), *work),
            _started_tasks.end());
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() removing finished work"
                  << std::endl;
#endif
        _finished_tasks.push_back(*work);
      }
    }
  }

  void schedule_task() {
#if DO_LOG
    std::cout << "Executor::schedule_task() size: " << _queued_tasks.size()
              << std::endl;
#endif
    std::unique_lock<std::mutex> lock(*_mutex);
    auto queued_tasks_iterator = _queued_tasks.begin();
    while (queued_tasks_iterator != _queued_tasks.end()) {
      if (queued_tasks_iterator->get()->can_be_started()) {
        _scheduled_tasks.insert(_scheduled_tasks.begin(),
                                *queued_tasks_iterator);
        queued_tasks_iterator = _queued_tasks.erase(queued_tasks_iterator);
      } else
        queued_tasks_iterator++;
    }
  }

  std::optional<Task> pop_task() {
#if DO_LOG
    std::cout << "Executor::pop_task()" << std::endl;
#endif
    std::unique_lock<std::mutex> lock(*_mutex);
    if (_scheduled_tasks.empty()) {
      return std::nullopt;
    }
    auto work = _scheduled_tasks.back();
    _scheduled_tasks.pop_back();
    _started_tasks.push_back(work);
    return work;
  }

  void cancel_all() {
    std::unique_lock<std::mutex> lock(*_mutex);
    _cancelled = true;
  }

  std::thread _main_thread;
  std::vector<std::thread> _worker_threads;
  std::vector<Task> _queued_tasks;
  std::vector<Task> _scheduled_tasks;
  std::vector<Task> _started_tasks;
  std::vector<Task> _finished_tasks;
  bool _cancelled = false;
  std::shared_ptr<std::mutex> _mutex = std::make_shared<std::mutex>();
};

} // namespace par