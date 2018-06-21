#include <algorithm>
#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <vector>

namespace {

template <typename T>
std::map<T*, std::vector<void(T::*)(int)>> g_map;

class Interface {
 public:
  virtual void foo(int) = 0;
};

class Inh : public Interface {
 public:
  void foo(int) override {
    std::cout << "Inh::foo called." << std::endl;
    auto iter = g_map<Inh>.find(this);
    if (iter == g_map<Inh>.end()) {
      std::cerr << "Not expected mock function called." << std::endl;
      return;
    }
    auto elem = std::find(std::begin(iter->second), std::end(iter->second), &Inh::foo);
    if (elem == iter->second.end()) {
      std::cerr << "Not expected mock function called." << std::endl;
      return;
    }
    iter->second.erase(elem);
  }
};

template<typename T, typename F>
class ExpectCall {
 public:
  ExpectCall(T* object, F funptr) : object_(object), funptr_(funptr) {
    auto iter = g_map<T>.find(object);
    if (iter == g_map<T>.end()) {
      std::vector<void(T::*)(int)> vec;
      vec.push_back(funptr);
      g_map<T>[object] = vec;
    } else {
      iter->second.push_back(funptr);
    }
  }

  ~ExpectCall() {
    auto iter = g_map<T>.find(object_);
    assert(iter != g_map<T>.end());
    for (const auto& ptr : iter->second) {
      if (ptr == funptr_) {
        std::cerr << "Expected call was not called." << std::endl;
      }
    }
  }

 private:
  T* object_;
  F funptr_;
};

#define CONC(s1, s2) s1 ## s2
#define CONC2(s1, s2) CONC(s1, s2)
#define UNIQUE_NAME CONC2(UN_, __LINE__)

#define EXPECT_CALL(obj, fun) ExpectCall<decltype(obj), decltype(&decltype(obj)::fun)> UNIQUE_NAME(&obj, &decltype(obj)::fun)

}  // unnamed namespace


int main() {
  Inh inh;
  EXPECT_CALL(inh, foo);
  //EXPECT_CALL(inh, foo);
  //inh.foo(1);
  return 0;
}
