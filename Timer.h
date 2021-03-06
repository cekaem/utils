#ifndef TIMER_H
#define TIMER_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <chrono>

namespace utils {

class Timer {
 public:
  ~Timer() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  bool start(unsigned time_ms, std::function<void()> callback) {
    if (timer_started_ == true) {
      return false;
    }
    timer_started_ = true;
    thread_ = std::thread(&Timer::timerMain, this, time_ms, callback);
    return true;
  }

  void stop() {
    timer_started_ = false;
    cv_.notify_one();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  void timerMain(unsigned time_ms, std::function<void()> callback) {
    std::mutex m;
    std::unique_lock ul(m);
    using namespace std::chrono_literals;
    auto status = cv_.wait_for(ul, time_ms * 1ms);
    if (status == std::cv_status::timeout) {
      callback();
    }
  }
 
  bool timer_started_{false};
  std::thread thread_;
  std::condition_variable cv_;
};

}  // namespace utils

#endif  // TIMER_H
