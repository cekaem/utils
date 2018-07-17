#ifndef SOCKET_LOG_H
#define SOCKET_LOG_H

#include <condition_variable>
#include <mutex>
#include <ostream>
#include <sstream>

namespace utils {

class SocketLog {
 public:
  using SocketLogManipulator = Log&(*)(Log&);

  ~SocketLog();

  void waitForClient(unsigned port);

  template<typename T>
  Log& operator<<(const T& t) {
    buffer_empty_mutex_.lock();
    buffer_ << t;
    buffer_empty_mutex_.unlock();
    buffer_not_empty_cv_.notify_one();
    return *this;
  }

  Log& operator<<(LogManipulator manip) {
    return manip(*this);
  }

  static Log& endl(Log& l) {
    l << '\n';
    l.mutex_.unlock();
    return l;
  }

  static Log& lock(Log& l) {
    l.mutex_.lock();
    return l;
  }

  static Log& unlock(Log& l) {
    l.mutex_.unlock();
    return l;
  }

 private:
  void loggerMain();

  std::stringstream buffer_;
  std::condition_variable buffer_not_empty_cv_;
  std::mutex buffer_empty_mutex_;
  std::mutex mutex_;
  int socket_fd_{-1};
};

}  // namesapce utils

#endif  // SOCKET_LOG_H
