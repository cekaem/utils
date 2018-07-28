#include "Socket.h"

#include <iostream>

int main() {
  try {
    utils::Socket socket(9090);
  } catch (const utils::Socket::SocketException& e) {
    std::cerr << e.error_message << std::endl;
  }
  return 0;
}
