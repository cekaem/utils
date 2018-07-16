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

Socket::Socket(unsigned port) {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    throw SocketException(std::string("Error during creation of socket: ") + strerror(errno));
  }

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  int error_code = bind(socket_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
  if (error_code < 0) {
    throw SocketException(std::string("Error during binding socket: ") + strerror(errno));
  }

  error_code = listen(socket_fd, 3);
  if (error_code < 0) {
    throw SocketException(std::string("Error during marking socket as a passive socket: ") + strerror(errno));
  }
  
  int addrlen = sizeof(address);
  socket_descriptor_ = accept(socket_fd,
                              reinterpret_cast<sockaddr*>(&address),
                              reinterpret_cast<socklen_t*>(&addrlen));
  if (socket_descriptor_ < 0) {
    throw SocketException(std::string("Error during creation of connected socket: ") + strerror(errno));
  }
  while (1) {
    ::write(socket_descriptor_, "dupa\n", 5);
    sleep(1);
  }
}

Socket::~Socket() {
  std::cerr << "Closing socket" << std::endl;
  close(socket_descriptor_);
}
