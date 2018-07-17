#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <ostream>

namespace utils {

class Log {
 public:
  using LogManipulator = Log&(*)(Log&);

  Log(std::ostream& ostr) : ostr_(ostr) {}

  template<typename T>
  Log& operator<<(const T& t) {
    ostr_ << t;
    return *this;
  }

  Log& operator<<(LogManipulator manip) {
    return manip(*this);
  }

  static Log& endl(Log& l) {
    l.ostr_ << std::endl;
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
  std::ostream& ostr_;
  std::mutex mutex_;
};


}  // namesapce utils

#endif  // LOG_H
