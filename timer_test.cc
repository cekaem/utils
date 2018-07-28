#include <functional>
#include <iostream>
#include <unistd.h>

#include "Timer.h"

class TimerTest {
 public:
  void callback() {
    std::cout << "callback called" << std::endl;
  }

  void startTimer() {
    std::function<void()> f = std::bind(&TimerTest::callback, this);
    timer_.start(1500, f);
  }

  void stopTimer() {
    timer_.stop();
  }

 private:
  utils::Timer timer_;
};


int main() {
  TimerTest test;
  test.startTimer();
  sleep(1);
  test.stopTimer();
  test.startTimer();
  sleep(3);
  return 0;
}
