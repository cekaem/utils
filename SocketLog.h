#ifndef SOCKET_LOG_H
#define SOCKET_LOG_H

#include <condition_variable>
#include <mutex>
#include <ostream>
#include <sstream>

namespace utils {

class SocketLog {
 public:
  using SocketLogManipulator = SocketLog&(*)(SocketLog&);

  void waitForClient(unsigned port);

  template<typename T>
  SocketLog& operator<<(const T& t) {
    buffer_empty_mutex_.lock();
    buffer_ << t;
    buffer_empty_mutex_.unlock();
    buffer_not_empty_cv_.notify_one();
    return *this;
  }

  SocketLog& operator<<(SocketLogManipulator manip) {
    return manip(*this);
  }

  static SocketLog& endl(SocketLog& l) {
    l << '\n';
    l.mutex_.unlock();
    return l;
  }

  static SocketLog& lock(SocketLog& l) {
    l.mutex_.lock();
    return l;
  }

  static SocketLog& unlock(SocketLog& l) {
    l.mutex_.unlock();
    return l;
  }

  static SocketLog& endLogging(SocketLog& l) {
    l.end_logging_ = true;
    l.buffer_not_empty_cv_.notify_one();
    return l;
  }

 private:
  void loggerMain();

  std::stringstream buffer_;
  std::condition_variable buffer_not_empty_cv_;
  std::mutex buffer_empty_mutex_;
  std::mutex mutex_;
  int socket_fd_{-1};
  bool end_logging_{false};
};

}  // namesapce utils

#endif  // SOCKET_LOG_H
