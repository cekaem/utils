#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

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

template <typename TT = void>
std::string getNameForType() {
  return typeid(TT).name();
}

template <typename FT, typename ST, typename ...VT>
std::string getNameForType() {
  std::string result(getNameForType<FT>());
  result.append(getNameForType<ST, VT...>());
  return result;
}

template <typename RT, typename ...PT>
std::string getNameForType(RT(T::*fun_ptr)(PT...)) {
  std::string result(getNameForType<RT>());
  result.append(getNameForType<PT...>());
  return result;
}


template <typename T>
class CallsRegister {
 public:
  template <typename RT, typename ...PT>
  static void registerCall(const T* object, const std::string& name, RT(T::*fun_ptr)(PT...)) {
    std::string function_type(name);
    function_type.append(getNameForType(fun_ptr));
    std::cout << function_type << std::endl;
    const auto& iter = std::find_if(counter_.begin(), counter_.end(),
          [object, function_type](const auto& iter) -> bool {
            return iter.first.first == object &&
                   iter.first.second == function_type;
          });
    if (iter == counter_.end()) {
      counter_[std::make_pair(object, function_type)] = 0;
    } else {
      ++(iter->second);
    }
  }

 private:
  static std::map<std::pair<const T*, std::string>, size_t> counter_;
};

template <typename T>
std::map<std::pair<const T*, std::string>, size_t> CallsRegister<T>::counter_;

template <typename T>
class CallsChecker{
 public:
  CallsChecker(const T* object, RT(T::*fun_ptr)(PT...)
};

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define EXPECT_CALL(__ptr__, __expected_calls_no__) CallsChecker MACRO_CONCAT( __cc__, __COUNTER__ )(__ptr__, __expected_calls_no__);

#define MOCK_BODY(_name_, _type_)\
  using this_type = std::remove_reference<decltype(*this)>::type;\
  CallsRegister<this_type>::registerCall(this, #_name_, &this_type::_name_);
#define MOCK_BEGIN()

#define MOCK_METHOD0(_name_, _type_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_() {\
    MOCK_BODY(_name_, _type_);\
  }

#define MOCK_METHOD0_WITH_RETURN_VALUE(_name_, _type_, _return_value_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_() {\
    MOCK_BODY(_name_, _type_);\
    return _return_value_;\
  }

#define MOCK_METHOD1(_name_, _type_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1) {\
    MOCK_BODY(_name_, _type_);\
  }

#define MOCK_METHOD1_WITH_RETURN_VALUE(_name_, _type_, _return_value_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1) {\
    MOCK_BODY(_name_, _type_);\
    return _return_value_;\
  }

#define MOCK_METHOD2(_name_, _type_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2) {\
    MOCK_BODY(_name_, _type_);\
  }

#define MOCK_METHOD2_WITH_RETURN_VALUE(_name_, _type_, _return_value_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2,\
      ) {\
    MOCK_BODY(_name_, _type_);\
    return _return_value_;\
  }

#define MOCK_METHOD3(_name_, _type_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2,\
      function_traits<std::function<_type_>>::arg<2>::type arg3) {\
    MOCK_BODY(_name_, _type_);\
  }

#define MOCK_METHOD3_WITH_RETURN_VALUE(_name_, _type_, _return_value_)\
  MOCK_BEGIN();\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2,\
      function_traits<std::function<_type_>>::arg<2>::type arg3) {\
    MOCK_BODY(_name_, _type_);\
    return _return_value_;\
  }

namespace {

class Base {
 public:
  virtual void fun_with_zero_params() = 0;
  virtual int fun_with_one_param(int n) = 0;
  virtual void fun_with_two_params(int n, double d) = 0;
  virtual float fun_with_three_params(int n, char* c, const std::vector<int>& vec) = 0;
};

class MockBase : public Base {
 public:
  MOCK_METHOD0(fun_with_zero_params, void());
  MOCK_METHOD1_WITH_RETURN_VALUE(fun_with_one_param, int(int), 0);
  MOCK_METHOD2(fun_with_two_params, void(int, double));
  MOCK_METHOD3_WITH_RETURN_VALUE(fun_with_three_params, float(int, char*, const std::vector<int>&), 2.0f);
};

}  // unnamed namespace

int main() {
  MockBase mb;
  EXPECT_CALL(&mb::fun_with_one_param, 1);
  EXPECT_CALL(&mb::fun_with_zero_params, 1);
  mb.fun_with_zero_params();
  mb.fun_with_one_param(5);
  std::vector<int> vec;
  mb.fun_with_three_params(6, nullptr, vec);
  return 0;
}
