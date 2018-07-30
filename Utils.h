#ifndef UTILS_H
#define UTILS_H
#include <string>

namespace utils {

bool str_2_uint(const std::string& str, unsigned& result);
void ltrim(std::string& str);
unsigned getMemoryUsageOfCurrentProcess();

}  // namespace utils

#endif  // UTILS_H
