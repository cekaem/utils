#ifndef TEST_H
#define TEST_H

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& ostr, const std::pair<T1, T2>& pair) {
  ostr << "(" << pair.first << ", " << pair.second << ")";
  return ostr;
}

}  // unnamed namespace


class Test {
 public:
  struct TestFailedException {};
  struct EndTestException {};

  Test() {
    error_line_ = 0;
    error_message_.clear();
    current_test_status_ = true;
  }

  static void Check(const char* name, bool is_ok) {
    std::cout << name << ": ";
    if (is_ok == true) {
      std::cout << "OK" << std::endl;
    } else {
      std::cout << "FAILED (line: " << error_line_ << ")" << std::endl;
      if (!error_message_.empty()) {
        std::cout << "Error message: " << error_message_ << std::endl;
      }
    }
    std::cout << std::endl;
  }

  static void SetErrorMessage(const std::string& error_message) {
    error_message_ = error_message;
  }

  static void Verify(bool expr, int line) {
    if (!expr) {
      error_line_ = line;
      throw TestFailedException();
    }
  }

  static int error_line_;
  static std::string error_message_;
  static bool current_test_status_;
};

template <typename T1, typename T2>
class TestHelper {
 public:
  TestHelper(const T1& t1, const T2& t2, int line)
    : is_ok_(t1 == t2), line_(line) {
    if (!is_ok_) {
      std::stringstream ss;
      ss << t1 << " vs " << t2 << std::endl;
      Test::error_message_ = ss.str();
    }
  }

  ~TestHelper() noexcept(false) {
    if (!is_ok_) {
      Test::error_line_ = line_;
      Test::error_message_ += error_messsage_stream_.str();
      throw Test::TestFailedException();
    }
  }

  std::stringstream& stream() {
    return error_messsage_stream_;
  }

 private:
  bool is_ok_{true};
  int line_{-1};
  std::stringstream error_messsage_stream_;
};

void VerifyStringsEqual(const char* str1, const char* str2, int line);

template <typename T>
void VerifyIsNull(const T* ptr1, int line) {
  if (ptr1 != nullptr) {
    Test::error_line_ = line;
    std::stringstream ss;
    ss << "Pointer is not null: " << ptr1;
    Test::SetErrorMessage(ss.str());
    throw Test::TestFailedException();
  }
}

template <typename T>
void VerifyIsNotNull(const T* ptr1, int line) {
  if (ptr1 == nullptr) {
    Test::error_line_ = line;
    std::stringstream ss;
    ss << "Pointer is null.";
    Test::SetErrorMessage(ss.str());
    throw Test::TestFailedException();
  }
}

template <typename T>
void VerifyContains(const std::vector<std::unique_ptr<T>>& vec, const T* value, int line) {
  auto iter = std::find_if(vec.begin(), vec.end(),
      [value](const auto& iter) -> bool {
        return iter.get() == value;
      });
  if (iter == std::end(vec)) {
    Test::error_line_ = line;
    throw Test::TestFailedException();
  }
}

template <typename T>
void VerifyContains(const std::vector<T>& vec, const T& value, int line) {
  auto iter = std::find(vec.begin(), vec.end(), value);
  if (iter == std::end(vec)) {
    Test::error_line_ = line;
    throw Test::TestFailedException();
  }
}

template <typename T>
void VerifyDoesNotContain(const std::vector<T>& vec, const T& value, int line) {
  auto iter = std::find(vec.begin(), vec.end(), value);
  if (iter != std::end(vec)) {
    Test::error_line_ = line;
    throw Test::TestFailedException();
  }
}

class TestProceduresMapAdder {
 public:
  static std::map<std::string, bool(*)()>* g_test_procedures;

  TestProceduresMapAdder(const std::string& name, bool(*ptr)()) {
    static std::map<std::string, bool(*)()> test_procedures;
    g_test_procedures = &test_procedures;
    test_procedures[name] = ptr;
  }
};

#define CONC2(s1, s2) s1 ## s2
#define CONC(s1, s2) CONC2(s1, s2)
#define UNIQUE_NAME CONC(UN_, __COUNTER__)


#define TEST_PROCEDURE(T)\
  bool T();\
  TestProceduresMapAdder UNIQUE_NAME(#T, &T);\
  bool T()

#define TEST_START \
  try { \
    Test test;

#define VERIFY(expr) test.Verify(expr, __LINE__)
#define VERIFY_TRUE(expr) TestHelper(expr, true, __LINE__).stream()
#define VERIFY_FALSE(expr) TestHelper(expr, false, __LINE__).stream()
#define VERIFY_EQUALS(val1, val2) TestHelper(val1, val2, __LINE__).stream()
#define VERIFY_STRINGS_EQUAL(expr1, expr2) VerifyStringsEqual(expr1, expr2, __LINE__)
#define VERIFY_CONTAINS(container, value) VerifyContains(container, value, __LINE__)
#define VERIFY_DOES_NOT_CONTAIN(container, value) VerifyDoesNotContain(container, value, __LINE__)
#define VERIFY_IS_NULL(ptr) VerifyIsNull(ptr, __LINE__)
#define VERIFY_IS_NOT_NULL(ptr) VerifyIsNotNull(ptr, __LINE__)
#define VERIFY_IS_ZERO(expr) VERIFY_IS_EQUAL(expr, 0)
#define SET_TEST_FAILED() Test::current_test_status_ = false;

#define TEST_END \
  }\
  catch (Test::TestFailedException& except) {\
    return false;\
  }\
  catch (Test::EndTestException& e) {\
  }\
  return Test::current_test_status_;

#define RETURN throw Test::EndTestException();

#define NOT_REACHED(msg) Test::SetErrorMessage(msg);\
                         VERIFY(1 == 0);

#endif // TEST_H
