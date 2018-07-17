#include "Socket.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace utils {

int Socket::acceptConnection() {
  int socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0) {
    throw SocketException(std::string("Error during creation of socket: ") + strerror(errno));
  }

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port_);
  int error_code = bind(socket_fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address));
  if (error_code < 0) {
    throw SocketException(std::string("Error during binding socket: ") + strerror(errno));
  }

  error_code = listen(socket_fd_, 3);
  if (error_code < 0) {
    throw SocketException(std::string("Error during marking socket as a passive socket: ") + strerror(errno));
  }
  
  int addrlen = sizeof(address);
  int result = accept(socket_fd_,
                      reinterpret_cast<sockaddr*>(&address),
                      reinterpret_cast<socklen_t*>(&addrlen));
  if (result < 0) {
    throw SocketException(std::string("Error during creation of connected socket: ") + strerror(errno));
  }
  return result;
}

Socket::~Socket() {
  close(socket_fd_);
}

} // namespace utils
