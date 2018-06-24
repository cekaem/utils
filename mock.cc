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

template <typename T>
std::string getNameForType() {
  return typeid(T).name();
}

template <typename T, typename ARG1>
class MockMap {
 public:
  std::map<T*, std::vector<void(T::*)(ARG1)>> map_;
  ~MockMap() {
    for (const auto& vec : map_) {
      for (const auto& mock: vec.second) {
        std::cerr << "Expected call was not called: " << getNameForType<decltype(mock)>() << std::endl;
      }
    }
  }
};

template <typename T, typename S>
MockMap<T, S> g_map;

class Interface {
 public:
  virtual void foo(int) = 0;
  virtual void foo2(double) = 0;
};

#define MOCK_CLASS(_name_)

#define MOCK_METHOD1(_name_, _type_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    auto iter = g_map<this_type, arg1_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type>.map_.end()) {\
      std::cerr << "Not expected mock function called: "<< getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      return;\
    }\
    auto elem = std::find(std::begin(iter->second), std::end(iter->second), &this_type::_name_);\
    if (elem == iter->second.end()) {\
      std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      return;\
    }\
    iter->second.erase(elem);\
  }\
  \
  void MOCK_##_name_(function_traits<std::function<_type_>>::arg<0>::type arg1) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    auto iter = g_map<this_type, arg1_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type>.map_.end()) {\
      std::vector<decltype(&this_type::_name_)> vec;\
      vec.push_back(&this_type::_name_);\
      g_map<this_type, arg1_type>.map_[this] = vec;\
    } else {\
      iter->second.push_back(&this_type::_name_);\
    }\
  }


class Inh : public Interface {
 public:
  MOCK_CLASS(Inh);
  MOCK_METHOD1(foo, void(int));
  MOCK_METHOD1(foo2, void(double));
};

#define CONC(s1, s2) s1 ## s2
#define CONC2(s1, s2) CONC(s1, s2)
#define UNIQUE_NAME CONC2(UN_, __LINE__)

//#define EXPECT_CALL(obj, fun) ExpectCall<decltype(obj), decltype(&decltype(obj)::fun)> UNIQUE_NAME(&obj, &decltype(obj)::fun)
#define EXPECT_CALL(obj, fun) obj.MOCK_##fun

}  // unnamed namespace

int main() {
  Inh inh;
  EXPECT_CALL(inh, foo(5));
  EXPECT_CALL(inh, foo2(7.0));
  inh.foo(1);
  inh.foo2(2.0);

  return 0;
}
