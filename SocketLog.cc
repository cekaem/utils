#include "SocketLog.h"

#include <thread>

#include "Socket.h"

namespace utils {

void SocketLog::waitForClient(unsigned port) {
  Socket socket(port);
  socket_fd_ = socket.acceptConnection();
  std::thread t(&SocketLog::loggerMain, this);
  t.detach();
}

void SocketLog::loggerMain() {
  std::string welcome_message("Logger started.\n");
  ::write(socket_fd_, welcome_message.c_str(), welcome_message.size());
  while (1) {
    std::unique_lock<std::mutex> ul(buffer_empty_mutex_);
    buffer_not_empty_cv_.wait(ul, [buffer_] { return buffer_.str().empty() == false; });
    mutex_.lock();
    std::string message = buffer_.str();
    buffer_.str(std::string());
    mutex_.unlock();
    ::write(socket_fd_, buffer_.c_str(), buffer.size());
  }
}

}  // namespace utils
