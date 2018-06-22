#include <algorithm>
#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <vector>

namespace {

template <typename T>
struct function_traits;

template<typename RT, typename ...PT>
struct function_traits<std::function<RT(PT...)>> {
  static constexpr size_t number_of_args = sizeof...(PT);
  using result_type = RT;

  template <size_t parameter_number>
  struct arg {
    using type = typename std::tuple_element<parameter_number, std::tuple<PT...>>::type;
  };
};

template <typename T, typename ARG1>
std::map<T*, std::vector<void(T::*)(ARG1)>> g_map;

template <typename T>
std::string getNameForType() {
  return typeid(T).name();
}

class Interface {
 public:
  virtual void foo(int) = 0;
  virtual void foo2(double) = 0;
};

#define MOCK_METHOD1(_name_, _type_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = function_traits<std::function<_type_>>::arg<0>::type;\
    auto iter = g_map<this_type, arg1_type>.find(this);\
    if (iter == g_map<this_type, arg1_type>.end()) {\
      std::cerr << "Not expected mock function called: "<< getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      return;\
    }\
    auto elem = std::find(std::begin(iter->second), std::end(iter->second), &this_type::_name_);\
    if (elem == iter->second.end()) {\
      std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      return;\
    }\
    iter->second.erase(elem);\
  }


class Inh : public Interface {
 public:
  MOCK_METHOD1(foo, void(int));
  MOCK_METHOD1(foo2, void(double));
};

template<typename T, typename F>
class ExpectCall {
 public:
  ExpectCall(T* object, F funptr) : object_(object), funptr_(funptr) {
    //using funptr_type = decltype(object->*funptr);
    using arg1_type = typename function_traits<decltype(*(object->funptr))>::template arg<0>::type;
    /*
    auto iter = g_map<T, arg1_type>.find(object);
    if (iter == g_map<T, arg1_type>.end()) {
      std::vector<funptr_type> vec;
      vec.push_back(funptr);
      g_map<T, arg1_type>[object] = vec;
    } else {
      iter->second.push_back(funptr);
    }
    */
  }

  ~ExpectCall() {
    /*
    using funptr_type = decltype(funptr_);
    using arg1_type = function_traits<std::function<funptr_type>>::arg<0>::type;
    auto iter = g_map<T, arg1_type>.find(object_);
    assert(iter != g_map<T, arg1_type>.end());
    for (const auto& ptr : iter->second) {
      if (ptr == funptr_) {
        std::cerr << "Expected call was not called: " << getNameForType<F>() << std::endl;
      }
    }
    */
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
  EXPECT_CALL(inh, foo2);
  //inh.foo(1);
  //inh.foo(2.0);
  return 0;
}
