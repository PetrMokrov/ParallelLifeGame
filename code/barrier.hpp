#pragma once

#include <condition_variable>
#include <mutex>


/*class Barrier {
 public:
  explicit Barrier(const size_t num_threads)
      : thread_count_{num_threads}, thread_pass_1_{int(num_threads)}, thread_pass_2_{int(num_threads)} {
  }


  void PassThrough() {
    std::unique_lock<std::mutex> lock{mutex_};
    --thread_pass_1_;

    // this means, that cv_1_ has been passed
    if(thread_pass_1_ < 0) {

      // so the barrier we try to pass become cv_2_
      --thread_pass_2_;
      if(thread_pass_2_ == 0) {

        // here we successfully pass through cv_2_,
        // and activate cv_1_
        thread_pass_1_ = thread_count_;
        cv_2_.notify_all();
      } else {
        cv_2_.wait(lock, [this]() { return thread_pass_2_ == 0; });
      }
      return;

    } else if (thread_pass_1_ == 0) {

      // here we activate cv_2_ condvar
      // and pass through cv_1_
      thread_pass_2_ = thread_count_;
      cv_1_.notify_all();

    } else {
      cv_1_.wait(lock, [this]() { return thread_pass_1_ == 0; });
    }
    return;
  }

 private:
  std::mutex mutex_;
  std::condition_variable cv_1_;
  std::condition_variable cv_2_;
  size_t thread_count_;
  int thread_pass_1_;
  int thread_pass_2_;
};*/

class Barrier {
 public:
  Barrier(const size_t num_threads)
      : num_threads_(num_threads), thread_count_(num_threads) {
  }

  void PassThrough() {
    std::unique_lock<std::mutex> lock(mutex_);
    if(gates_broken) {
      return;
    }
    --thread_count_;
    size_t curr_epoch = epoch_;
    if (thread_count_ == 0) {
      ++epoch_;
      thread_count_ = num_threads_;
      epoch_advanced_.notify_all();
    } else {
      while (curr_epoch == epoch_) {
        epoch_advanced_.wait(lock);
        if(gates_broken) {
          return;
        }
      }
    }
  }

  void BreakGates() {
    std::unique_lock<std::mutex> lock(mutex_);
    epoch_advanced_.notify_all();
    gates_broken = true;
  }

  void RecoverGates(){
    gates_broken = false;
  }

 private:
  size_t num_threads_;
  std::mutex mutex_;
  std::condition_variable epoch_advanced_;
  size_t thread_count_;
  size_t epoch_;
  bool gates_broken{false};
};