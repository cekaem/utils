#include <algorithm>
#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <utility>
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

template <typename T, typename ...ARGS>
class MockMap {
 public:
  std::map<T*, std::vector<std::tuple<std::string, void(T::*)(ARGS...), ARGS...>>> map_;
  ~MockMap() {
    for (const auto& elem : map_) {
      for (const auto& tuple: elem.second) {
        std::cerr << "Expected call was not called: " << std::get<0>(tuple) << std::endl;
        if constexpr(sizeof...(ARGS) > 0) {
          std::cerr << "with values: " << std::get<2>(tuple);
        }
        if constexpr(sizeof...(ARGS) > 1) {
          std::cerr << " ; " << std::get<3>(tuple);
        }
        if constexpr(sizeof...(ARGS) > 2) {
          std::cerr << " ; " << std::get<4>(tuple);
        }
        std::cerr << std::endl << std::endl;
      }
    }
  }
};

template <typename T, typename ...ARGS>
MockMap<T, ARGS...> g_map;

#define MOCK_METHOD0(_name_, _type_)\
  function_traits<std::function<_type_>>::result_type _name_() override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)()>(&this_type::_name_);\
    auto iter = g_map<this_type>.map_.find(this);\
    if (iter == g_map<this_type>.map_.end()) {\
      std::cerr << "Not expected mock function called: " << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      return;\
    }\
    auto elem = std::find_if(std::begin(iter->second),\
                             std::end(iter->second),\
                             [funptr](const auto& tuple) -> bool {\
                               return std::get<1>(tuple) == funptr;\
                             });\
    if (elem == iter->second.end()) {\
      std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      return;\
    }\
    iter->second.erase(elem);\
  }\
  \
  void MOCK_##_name_() {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)()>(&this_type::_name_);\
    auto iter = g_map<this_type>.map_.find(this);\
    if (iter == g_map<this_type>.map_.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr)>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr));\
      g_map<this_type>.map_[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr));\
    }\
  }

#define MOCK_METHOD1(_name_, _type_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)(arg1_type)>(&this_type::_name_);\
    auto iter = g_map<this_type, arg1_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type>.map_.end()) {\
      std::cerr << "Not expected mock function called: " << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      std::cerr << "with value: " << arg1 << std::endl << std::endl;\
      return;\
    }\
    auto elem = std::find_if(std::begin(iter->second),\
                             std::end(iter->second),\
                             [funptr, arg1](const auto& tuple) -> bool {\
                               return std::get<1>(tuple) == funptr &&\
                                      std::get<2>(tuple) == arg1;\
                             });\
    if (elem == iter->second.end()) {\
      std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      std::cerr << "with value: " << arg1 << std::endl << std::endl;\
      return;\
    }\
    iter->second.erase(elem);\
  }\
  \
  void MOCK_##_name_(function_traits<std::function<_type_>>::arg<0>::type arg1) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)(arg1_type)>(&this_type::_name_);\
    auto iter = g_map<this_type, arg1_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type>.map_.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr), arg1_type>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1));\
      g_map<this_type, arg1_type>.map_[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1));\
    }\
  }

#define MOCK_METHOD2(_name_, _type_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using arg2_type = decltype(arg2);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)(arg1_type, arg2_type)>(&this_type::_name_);\
    auto iter = g_map<this_type, arg1_type, arg2_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type, arg2_type>.map_.end()) {\
      std::cerr << "Not expected mock function called: " << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      std::cerr << "with values: " << arg1 << " ; " << arg2 << std::endl << std::endl;\
      return;\
    }\
    auto elem = std::find_if(std::begin(iter->second),\
                             std::end(iter->second),\
                             [funptr, arg1, arg2](const auto& tuple) -> bool {\
                               return std::get<1>(tuple) == funptr &&\
                                      std::get<2>(tuple) == arg1 &&\
                                      std::get<3>(tuple) == arg2;\
                             });\
    if (elem == iter->second.end()) {\
      std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      std::cerr << "with values: " << arg1 << " ; " << arg2 << std::endl << std::endl;\
      return;\
    }\
    iter->second.erase(elem);\
  }\
  \
  void MOCK_##_name_(function_traits<std::function<_type_>>::arg<0>::type arg1,\
                     function_traits<std::function<_type_>>::arg<1>::type arg2) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using arg2_type = decltype(arg2);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)(arg1_type, arg2_type)>(&this_type::_name_);\
    auto iter = g_map<this_type, arg1_type, arg2_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type, arg2_type>.map_.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr), arg1_type, arg2_type>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2));\
      g_map<this_type, arg1_type, arg2_type>.map_[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2));\
    }\
  }

#define MOCK_METHOD3(_name_, _type_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2,\
      function_traits<std::function<_type_>>::arg<2>::type arg3) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using arg2_type = decltype(arg2);\
    using arg3_type = decltype(arg3);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)(arg1_type, arg2_type, arg3_type)>(&this_type::_name_);\
    auto iter = g_map<this_type, arg1_type, arg2_type, arg3_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type, arg2_type, arg3_type>.map_.end()) {\
      std::cerr << "Not expected mock function called: " << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      std::cerr << "with values: " << arg1 << " ; " << arg2 << ", " << arg3 << std::endl << std::endl;\
      return;\
    }\
    auto elem = std::find_if(std::begin(iter->second),\
                             std::end(iter->second),\
                             [funptr, arg1, arg2, arg3](const auto& tuple) -> bool {\
                               return std::get<1>(tuple) == funptr &&\
                                      std::get<2>(tuple) == arg1 &&\
                                      std::get<3>(tuple) == arg2 &&\
                                      std::get<4>(tuple) == arg3;\
                             });\
    if (elem == iter->second.end()) {\
      std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      std::cerr << "with values: " << arg1 << " ; " << arg2 << " ; " << arg3 << std::endl << std::endl;\
      return;\
    }\
    iter->second.erase(elem);\
  }\
  \
  void MOCK_##_name_(function_traits<std::function<_type_>>::arg<0>::type arg1,\
                     function_traits<std::function<_type_>>::arg<1>::type arg2,\
                     function_traits<std::function<_type_>>::arg<2>::type arg3) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using arg2_type = decltype(arg2);\
    using arg3_type = decltype(arg3);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)(arg1_type, arg2_type, arg3_type)>(&this_type::_name_);\
    auto iter = g_map<this_type, arg1_type, arg2_type, arg3_type>.map_.find(this);\
    if (iter == g_map<this_type, arg1_type, arg2_type, arg3_type>.map_.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr), arg1_type, arg2_type, arg3_type>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2, arg3));\
      g_map<this_type, arg1_type, arg2_type, arg3_type>.map_[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2, arg3));\
    }\
  }


class Interface {
 public:
  virtual void foo() = 0;
  virtual void foo(int) = 0;
  virtual void foo(double) = 0;
  virtual void foo(int, double) = 0;
  virtual void foo(int, double, std::string) = 0;
};

class Inh : public Interface {
 public:
  MOCK_METHOD0(foo, void());
  MOCK_METHOD1(foo, void(int));
  MOCK_METHOD1(foo, void(double));
  MOCK_METHOD2(foo, void(int, double));
  MOCK_METHOD3(foo, void(int, double, std::string));
};

#define EXPECT_CALL(obj, fun) obj.MOCK_##fun

}  // unnamed namespace

int main() {
  Inh inh;
  EXPECT_CALL(inh, foo());
  EXPECT_CALL(inh, foo(1));
  EXPECT_CALL(inh, foo(2.0));
  EXPECT_CALL(inh, foo(5, 3.0));
  EXPECT_CALL(inh, foo(7, 8.0, "string"));
  inh.foo();
  inh.foo(1);
  inh.foo(2.0);
  inh.foo(5, 3.0);
  inh.foo(7, 8.0, "string");

  return 0;
}
