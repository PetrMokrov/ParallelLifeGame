#pragma once

#include <functional>
#include <thread>
#include <vector>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

class Executor {
  using Task = std::function<void(void)>;

 public:
  void Submit(Task task) {
    joined_ = false;
    task_threads_.emplace_back(&Executor::Execute, task);
  }

  template <typename... Args>
  void Submit(Args&&... args) {
    Submit(Task(std::bind(std::forward<Args>(args)...)));
  }

  ~Executor() {
    Join();
  }

  void Join() {
    if (joined_) {
      return;
    }
    for (auto& task_thread : task_threads_) {
      task_thread.join();
    }
    task_threads_.clear();
    joined_ = true;
  }

 private:
  static std::string CurrentExceptionMessage() {
    try {
      throw;
    } catch (const std::exception& e) {
      return e.what();
    } catch (...) {
      return "wild exception...";
    }
  }

  static std::string FormatCurrentExceptionMessage() {
    std::ostringstream out;
    out << "Uncaught exception in thread " << std::this_thread::get_id() << ": "
        << CurrentExceptionMessage();
    return out.str();
  }

  static void FailTestByException() {
    FailTest(FormatCurrentExceptionMessage());
  }

  static void FailTest(const std::string& error_message) {
    static std::mutex mutex;

    std::lock_guard<std::mutex> locked(mutex);
    std::cerr << error_message << std::endl;
    std::abort();
    
  }

  static void Execute(Task task) {
    try {
      task();
    } catch (...) {
      FailTestByException();
    }
  }

 private:
  std::vector<std::thread> task_threads_;
  bool joined_{false};
};