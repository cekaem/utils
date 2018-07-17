#include "SocketLog.h"

#include <thread>
#include <unistd.h>

#include "Socket.h"

namespace utils {

void SocketLog::waitForClient(unsigned port) {
  Socket socket(port);
  socket_fd_ = socket.acceptConnection();
  std::thread t(&SocketLog::loggerMain, this);
  t.detach();
}

void SocketLog::loggerMain() {
  std::string welcome_message("Logging started.\n");
  ::write(socket_fd_, welcome_message.c_str(), welcome_message.size());
  while (end_logging_ == false) {
    std::unique_lock<std::mutex> ul(buffer_empty_mutex_);
    buffer_not_empty_cv_.wait(ul, [this] { return buffer_.str().empty() == false || end_logging_ == true; });
    std::string message = buffer_.str();
    buffer_.str(std::string());
    ::write(socket_fd_, message.c_str(), message.size());
  }
  std::string end_message("Logging ended.\n");
  ::write(socket_fd_, end_message.c_str(), end_message.size());
  close(socket_fd_);
}

}  // namespace utils
