#include "Test.h"

#include <iostream>

#include "CommandLineParser.h"


int Test::error_line_ = 0;
std::string Test::error_message_;
bool Test::current_test_status_ = true;

std::map<std::string, bool(*)()>* TestProceduresMapAdder::g_test_procedures{nullptr};

void VerifyStringsEqual(const char* str1, const char* str2, int line) {
  if (std::string(str1) != std::string(str2)) {
    Test::error_line_ = line;
    std::stringstream ss;
    ss << str1 << " vs " << str2;
    Test::SetErrorMessage(ss.str());
    throw Test::TestFailedException();
  }
}


int main(int argc, char* argv[]) {
  //CommandLineParser parser;
  int number_of_failed_tests = 0u;
  if (TestProceduresMapAdder::g_test_procedures == nullptr) {
    return 0;
  }
  for (auto& iter: *TestProceduresMapAdder::g_test_procedures) {
    if (argc == 1 || (argc == 2 && iter.first == argv[1])) {
      bool result = (*iter.second)();
      std::cout << iter.first << ": " << (result == true ? "OK" : "FAILED") << std::endl;
      if (result == false) {
        ++number_of_failed_tests;
        std::cout << "line: " << Test::error_line_ << std::endl;
        if (!Test::error_message_.empty()) {
          std::cout << "Error message: " << Test::error_message_ << std::endl;
        }
        std::cout << std::endl;
      }
    }
  }
  std::cout << std::endl;
  if (number_of_failed_tests == 0) {
    std::cout << "All tests passed." << std::endl;
  } else {
    std::cout << number_of_failed_tests << " failed." << std::endl;
  }

  return number_of_failed_tests;
}
