#include "Socket.h"

#include <iostream>

int main() {
  try {
    Socket socket(9090);
  } catch (const Socket::SocketException& e) {
    std::cerr << e.error_message << std::endl;
  }
  return 0;
}
