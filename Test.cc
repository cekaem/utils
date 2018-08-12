#include "Test.h"

#include <iostream>
#include <string>
#include <utility>
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

std::vector<std::pair<std::string, bool(*)()>> generateListOfTestsToLaunch(std::string tests) {
  std::vector<std::pair<std::string, bool(*)()>> result;
  if (TestProceduresMapAdder::g_test_procedures == nullptr) {
    return result;
  }
  if (tests.empty() == true) {
    for (const auto& test_procedure: *TestProceduresMapAdder::g_test_procedures) {
      result.push_back(std::make_pair(test_procedure.first, test_procedure.second));
    }
    return result;
  }

  std::vector<std::string> tests_from_cmd;
  auto pos = std::string::npos;
  while ((pos = tests.find(':')) != std::string::npos) {
    tests_from_cmd.push_back(tests.substr(0, pos));
    if (tests.size() == pos) {
      tests.clear();
    } else {
      tests = tests.substr(pos + 1);
    }
  }
  if (tests.empty() == false) {
    tests_from_cmd.push_back(tests);
  }

  for (const auto& test: tests_from_cmd) {
    auto iter = TestProceduresMapAdder::g_test_procedures->find(test);
    if (iter != TestProceduresMapAdder::g_test_procedures->end()) {
      result.push_back(std::make_pair(iter->first, iter->second));
    }
  }
  return result;
}


}  // unnamed namespace


int main(int argc, const char* argv[]) {
  int repeat_time = 0;
  std::string tests_to_launch_str;
  try {
    CommandLineParser parser;
    parser.addIntegerParameter('c', 1);
    parser.addStringParameter('t');
    parser.parse(argc, argv);
    repeat_time = parser.getIntegerValue('c');
    tests_to_launch_str = parser.getStringValue('t');
  } catch (CommandLineParser::CommandLineParserGeneralException&) {
    std::cerr << "Usage: " << argv[0] << " [-c repeat_time] [-t tests_to_launch]" << std::endl;
    return -1;
  }

  auto tests_to_launch = generateListOfTestsToLaunch(tests_to_launch_str);
  int number_of_failed_tests = 0u;
  for (int i = 0; i < repeat_time; ++i) {
    for (auto& iter: tests_to_launch) {
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
  if (tests_to_launch.empty() == false && number_of_failed_tests == 0) {
    std::cout << "All tests passed." << std::endl;
  } else if (number_of_failed_tests > 0) {
    std::cout << number_of_failed_tests << " failed." << std::endl;
  }

  return number_of_failed_tests;
}
