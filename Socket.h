#ifndef SOCKET_H
#define SOCKET_H

#include <exception>
#include <string>

class Socket {
 public:
  struct SocketException : std::exception {
    SocketException(const std::string& message) : error_message(message) {}
    const std::string error_message;
  };

  Socket(unsigned port);
  ~Socket();
  void write(const std::string& str);

 private:
  int socket_descriptor_{-1};
};

#endif  // SOCKET_H
