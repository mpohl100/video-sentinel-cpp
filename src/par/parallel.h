#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#define DO_LOG 0

namespace par {

class Work;

class Task {
public:
  Task() = default;
  Task(const Task &) = default;
  Task(Task &&) = default;
  Task &operator=(const Task &) = default;
  Task &operator=(Task &&) = default;
  virtual ~Task() = default;
  Task(Work *work) : _work{work} {}

  void precede(Task &task);

private:
  Work *_work;
};

class Work {
public:
  Work() = default;
  Work(const Work &) = default;
  Work(Work &&) = default;
  Work &operator=(const Work &) = default;
  Work &operator=(Work &&) = default;
  virtual ~Work() = default;

  virtual void call() = 0;
  Task make_task() { return Task{this}; }
  void add_predecessor(Work *work) { _predecessors.push_back(work); }
  bool can_be_started() const {
    return std::all_of(_predecessors.cbegin(), _predecessors.cend(),
                        [](Work *work) { return work->is_finished(); });
  }
  bool is_finished() const { return _finished; }
  void set_finished() { _finished = true; }

private:
  std::vector<Work *> _predecessors;
  bool _finished = false;
};

inline void Task::precede(Task &task) {
#if DO_LOG
  std::cout << "Task::precede()" << std::endl;
#endif
  task._work->add_predecessor(_work);
}

class Calculation : public Work {
public:
  Calculation(std::function<void()> func) : Work{}, _func{func} {}
  Calculation() = default;
  Calculation(const Calculation &) = default;
  Calculation(Calculation &&) = default;
  Calculation &operator=(const Calculation &) = default;
  Calculation &operator=(Calculation &&) = default;
  virtual ~Calculation() = default;

  void call() override {
#if DO_LOG
    std::cout << "Calculation::call()" << std::endl;
#endif
    _func();
  }

  std::function<void()> _func;
};

class Flow : public Work {
public:
  Flow() : Work{}, _work{} {}
  Flow(Flow &&) = default;
  Flow &operator=(Flow &&) = default;
  virtual ~Flow() = default;

  void add(std::unique_ptr<Work> work) { _work.push_back(std::move(work)); }

  void call() override {
#if DO_LOG
    std::cout << "Flow::call()" << std::endl;
#endif
    for (auto &work : _work) {
      work->call();
    }
  }

private:
  std::vector<std::unique_ptr<Work>> _work;
};

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

  void run(Work *work) {
#if DO_LOG
    std::cout << "Executor::run()" << std::endl;
#endif
    std::unique_lock<std::mutex> lock(*_mutex);
    _queued_work.push_back(work);
  }

  void wait_for(Work *work) {
#if DO_LOG
    std::cout << "Executor::wait_for()" << std::endl;
#endif
    while (true) {
      bool do_break = false;
      {
#if DO_LOG
        std::cout << "Executor::wait_for() checking if finished" << std::endl;
#endif
        std::unique_lock<std::mutex> lock(*_mutex);
        auto it = std::find(_finished_work.begin(), _finished_work.end(), work);
        if (it != _finished_work.end()) {
          _finished_work.erase(it);
          do_break = true;
        }
      }
      if (do_break) {
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
      schedule_work();
      auto *work = pop_work();
      if (work == nullptr) {
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() sleep 1ms" << std::endl;
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      if (work) {
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() do_work" << std::endl;
#endif
        work->call();
        work->set_finished();
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() finished_work"
                  << std::endl;
#endif
        std::unique_lock<std::mutex> lock(*_mutex);
        _started_work.erase(
            std::remove(_started_work.begin(), _started_work.end(), work),
            _started_work.end());
#if DO_LOG
        std::cout << "Executor::execute_worker_thread() removing finished work"
                  << std::endl;
#endif
        _finished_work.push_back(work);
      }
    }
  }

  void schedule_work() {
#if DO_LOG
    std::cout << "Executor::schedule_work() size: " << _queued_work.size() << std::endl;
#endif
    std::unique_lock<std::mutex> lock(*_mutex);
    auto queued_work_iterator = _queued_work.begin();
    while(queued_work_iterator != _queued_work.end()) {
      if ((*queued_work_iterator)->can_be_started()) {
        _scheduled_work.insert(_scheduled_work.begin(), *queued_work_iterator);
        queued_work_iterator = _queued_work.erase(queued_work_iterator);
      }
      else
        queued_work_iterator++;
    }
  }

  Work *pop_work() {
#if DO_LOG
    std::cout << "Executor::pop_work()" << std::endl;
#endif
    std::unique_lock<std::mutex> lock(*_mutex);
    if (_scheduled_work.empty()) {
      return nullptr;
    }
    auto *work = _scheduled_work.back();
    _scheduled_work.pop_back();
    _started_work.push_back(work);
    return work;
  }

  void cancel_all() {
    std::unique_lock<std::mutex> lock(*_mutex);
    _cancelled = true;
  }

  std::thread _main_thread;
  std::vector<std::thread> _worker_threads;
  std::vector<Work *> _queued_work;
  std::vector<Work *> _scheduled_work;
  std::vector<Work *> _started_work;
  std::vector<Work *> _finished_work;
  bool _cancelled = false;
  std::shared_ptr<std::mutex> _mutex = std::make_shared<std::mutex>();
};

} // namespace par