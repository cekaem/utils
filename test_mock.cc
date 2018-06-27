#include "Mock.h"

namespace {

class Interface {
 public:
  virtual void foo() = 0;
  virtual void foo(int) = 0;
  virtual void foo2(double) = 0;
  virtual void foo(int, double) = 0;
  virtual void foo(int, double, std::string) = 0;
  virtual void foo(int, double, std::string, bool) = 0;
};

class Inh : public Interface {
 public:
  MOCK_CLASS(Inh)
  MOCK_METHOD0(foo, void());
  MOCK_METHOD1(foo, void(int));
  MOCK_METHOD1(foo2, void(double));
  MOCK_METHOD2(foo, void(int, double));
  MOCK_METHOD3(foo, void(int, double, std::string));
  MOCK_METHOD4(foo, void(int, double, std::string, bool));
};

}  // unnamed namespace


int main() {
  using AnyValue::_;
  Inh inh;
  EXPECT_CALL(inh, foo());
  EXPECT_CALL(inh, foo(5));
  EXPECT_CALL(inh, foo(_));
  EXPECT_CALL(inh, foo2(2.0));
  EXPECT_CALL(inh, foo(_, _));
  EXPECT_CALL(inh, foo(7, 8.0, _));
  EXPECT_CALL(inh, foo(1, 2.0, "three", false));
  inh.foo();
  inh.foo(3);
  inh.foo(5);
  inh.foo2(2.0);
  inh.foo(5, 8.0);
  inh.foo(7, 8.0, "string");
  inh.foo(1, 2.0, "three", false);

  return 0;
}
