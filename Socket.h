#ifndef SOCKET_H
#define SOCKET_H

#include <exception>
#include <string>

namespace utils {

class Socket {
 public:
  struct SocketException : std::exception {
    SocketException(const std::string& message) : error_message(message) {}
    const char* what() const noexcept override { return error_message.c_str(); }
    const std::string error_message;
  };

  Socket(unsigned port) : port_(port) {}
  ~Socket();
  int acceptConnection();

 private:
  unsigned port_{0};
  int socket_fd_{-1};
};

}  // namespace utils

#endif  // SOCKET_H
