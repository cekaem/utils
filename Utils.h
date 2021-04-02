#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>

namespace utils {

template <typename T>
bool str_2_number(const std::string& str, T& result) {
  std::stringstream ss(str);
  ss >> result;
  return ss.fail() == false && ss.bad() == false;
}

void ltrim(std::string& str);
unsigned getMemoryUsageOfCurrentProcess();

}  // namespace utils

#endif  // UTILS_H
