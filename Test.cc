#include "Test.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

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

namespace {

std::vector<std::string> extractTestsToLaunchFromString(std::string tests) {
  std::vector<std::string> tests_to_launch;

  auto pos = std::string::npos;
  while ((pos = tests.find(':')) != std::string::npos) {
    tests_to_launch.push_back(tests.substr(0, pos));
    if (tests.size() == pos) {
      tests.clear();
    } else {
      tests = tests.substr(pos + 1);
    }
  }
  if (tests.empty() == false) {
    tests_to_launch.push_back(tests);
  }

  return tests_to_launch;
}


}  // unnamed namespace


int main(int argc, const char* argv[]) {
  int repeat_time = 0;
  std::string tests_to_launch_str;
  bool list_tests = false;
  try {
    CommandLineParser parser;
    parser.addBinaryParameter('l');
    parser.addIntegerParameter('c', 1);
    parser.addStringParameter('t');
    parser.setLastParameterIsNotAllowed();
    parser.parse(argc, argv);
    list_tests = parser.getBinaryValue('l');
    repeat_time = parser.getIntegerValue('c');
    tests_to_launch_str = parser.getStringValue('t');
  } catch (CommandLineParser::CommandLineParserGeneralException&) {
    std::cerr << "Usage: " << argv[0] << " [-l] [-c repeat_time] [-t tests_to_launch]" << std::endl;
    return -1;
  }
  if (TestProceduresMapAdder::g_test_procedures == nullptr) {
    return 0;
  }

  if (list_tests  == true) {
    for (auto& iter: *TestProceduresMapAdder::g_test_procedures) {
      std::cout << iter.first << std::endl;
    }
    return 0;
  }

  auto tests_to_launch = extractTestsToLaunchFromString(tests_to_launch_str);
  int number_of_failed_tests = 0u;
  for (int i = 0; i < repeat_time; ++i) {
    for (auto& iter: *TestProceduresMapAdder::g_test_procedures) {
      if (tests_to_launch.empty() == false) {
        if (std::find(std::begin(tests_to_launch),
                      std::end(tests_to_launch),
                      iter.first) == std::end(tests_to_launch)) {
          continue;
        }
      }
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
  } else if (number_of_failed_tests > 0) {
    std::cout << number_of_failed_tests << " failed." << std::endl;
  }

  return number_of_failed_tests;
}
