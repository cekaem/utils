#include "Test.h"

#include <string>


namespace {

TEST_PROCEDURE(first_test) {
  TEST_START
  VERIFY(false);
  TEST_END
}

TEST_PROCEDURE(second_test) {
  TEST_START
  std::string s("ala");
  VERIFY_STRINGS_EQUAL("ala", s.c_str());
  TEST_END
}

}  // unnamed namespace
