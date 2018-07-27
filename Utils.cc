#include "Utils.h"

#include <algorithm>
#include <sstream>
#include <string>


namespace utils {

bool str_2_uint(const std::string& str, unsigned& result) {
  std::stringstream ss(str);
  ss >> result;
  return ss.fail() == false && ss.bad() == false;
}

void ltrim(std::string& str) {
  str.erase(std::begin(str), std::find_if(std::begin(str),
                                          std::end(str),
                                          [] (char c) -> bool { return std::isspace(c) == false; }));
}

}  // namespace utils
