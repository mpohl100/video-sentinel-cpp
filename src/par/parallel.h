#pragma once

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

inline void Task::succeed(Task &task) {
#if DO_LOG
  std::cout << "Task::precede()" << std::endl;
#endif
  _work->add_predecessor(task._work);
}

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