#include "Utils.h"

#include <algorithm>
#include <exception>
#include <sstream>
#include <stdio.h>
#include <string>

namespace {

class FileDescriptorWrapper {
 public:
  FileDescriptorWrapper(FILE* desc) : desc_(desc) {}
  ~FileDescriptorWrapper() {
    fclose(desc_);
  }

 private:
  FILE* desc_{nullptr};
};

}  // unnamed namespace

namespace utils {

void ltrim(std::string& str) {
  str.erase(std::begin(str), std::find_if(std::begin(str),
                                          std::end(str),
                                          [] (char c) -> bool { return std::isspace(c) == false; }));
}

unsigned getMemoryUsageOfCurrentProcess() {
  unsigned memory_consumption = 0u;
  FILE* desc = fopen("/proc/self/status", "r");
  if (desc == nullptr) {
    throw std::runtime_error("Can't open /proc/self/status");
  }
  FileDescriptorWrapper wrapper(desc);
  char buff[128];
  while (fgets(buff, 128, desc) != nullptr) {
    std::string line(buff);
    auto pos = line.find("VmRSS:");
    if (pos != 0) {
      continue;
    }
    std::string sub = line.substr(6);  // 6 == strlen("VmRSS:")
    utils::ltrim(sub);
    pos = sub.find(" kB");
    if (pos == std::string::npos) {
      throw std::runtime_error("Error during parsing /proc/self/status");
    }
    std::string memory_consumption_str = sub.substr(0, pos);
    if (utils::str_2_number(memory_consumption_str, memory_consumption) == false) {
      throw std::runtime_error("Error during reading memory consumption from /proc/self/status");
    }
  }
  if (memory_consumption == 0) {
    throw std::runtime_error("No information about VmRSS in /proc/self/status");
  }
  return memory_consumption;
}

}  // namespace utils
