#ifndef MOCK_H
#define MOCK_H
#include <algorithm>
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "Test.h"

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
std::map<T*, std::vector<std::tuple<std::string, void(T::*)(ARGS...), ARGS...>>> g_map;

template <typename T, typename ...ARGS>
class MockMapChecker {
 public:
  ~MockMapChecker() {
    for (const auto& elem : g_map<T, ARGS...>) {
      for (const auto& tuple: elem.second) {
        SET_TEST_FAILED();
        std::cerr << "Expected call was not called: " << std::get<0>(tuple) << std::endl;
        if constexpr(sizeof...(ARGS) > 0) {
          std::cerr << "with value(s): " << std::get<2>(tuple);
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
    g_map<T, ARGS...>.clear();
  }
};

class MockAnyValue {
 public:
  bool operator==(const MockAnyValue&) const {
    return true;
  }

  friend std::ostream& operator<<(std::ostream& ostr, const MockAnyValue&);
};

namespace AnyValue {
MockAnyValue _;
}

std::ostream& operator<<(std::ostream& ostr, const MockAnyValue&) {
  ostr << "AnyValue";
  return ostr;
}

#define CONC2(s1, s2) s1 ## s2
#define CONC(s1, s2) CONC2(s1, s2)
#define UNIQUE_NAME CONC(UN_, __COUNTER__)

#define MOCK_CLASS(_NAME_) using CLASS_TYPE = _NAME_;

#define MOCK_METHOD0(_name_, _type_)\
  MockMapChecker<CLASS_TYPE> UNIQUE_NAME;\
  function_traits<std::function<_type_>>::result_type _name_() override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    auto funptr = static_cast<result_type(this_type::*)()>(&this_type::_name_);\
    auto iter = g_map<this_type>.find(this);\
    if (iter == g_map<this_type>.end()) {\
      SET_TEST_FAILED();\
      std::cerr << "Not expected mock function called: " << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
      return;\
    }\
    auto elem = std::find_if(std::begin(iter->second),\
                             std::end(iter->second),\
                             [funptr](const auto& tuple) -> bool {\
                               return std::get<1>(tuple) == funptr;\
                             });\
    if (elem == iter->second.end()) {\
      SET_TEST_FAILED();\
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
    auto iter = g_map<this_type>.find(this);\
    if (iter == g_map<this_type>.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr)>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr));\
      g_map<this_type>[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr));\
    }\
  }

#define MOCK_BODY1(_name_, _result_type_, _arg1_type_)\
  void MOCK_##_name_(_arg1_type_ arg1) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    auto funptr = static_cast<_result_type_(this_type::*)(_arg1_type_)>(&this_type::_name_);\
    auto iter = g_map<this_type, _arg1_type_>.find(this);\
    if (iter == g_map<this_type, _arg1_type_>.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr), _arg1_type_>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1));\
      g_map<this_type, _arg1_type_>[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1));\
    }\
  }

#define EMPTY_MOCK1(_name_, _result_type_, _arg1_type_)\
  void _name_(_arg1_type_ arg1) {}\

#define FIND_ELEMENT_IN_MOCKS_MAP1(_name_)\
template <typename OT, typename RT, typename AT>\
bool find_element_in_mocks_map_##_name_(OT* object, AT arg1) {\
  auto funptr = static_cast<RT(OT::*)(AT)>(&OT::_name_);\
  auto iter = g_map<OT, AT>.find(object);\
  if (iter == g_map<OT, AT>.end()) {\
    return false;\
  }\
  auto elem = std::find_if(std::begin(iter->second),\
                           std::end(iter->second),\
                           [funptr, arg1](const auto& tuple) -> bool {\
                             return std::get<1>(tuple) == funptr &&\
                                    std::get<2>(tuple) == arg1;\
                           });\
  if (elem == iter->second.end()) {\
    return false;\
  }\
  iter->second.erase(elem);\
  return true;\
}

#define MOCK_METHOD1(_name_, _type_)\
  MockMapChecker<CLASS_TYPE, function_traits<std::function<_type_>>::arg<0>::type> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE, MockAnyValue> UNIQUE_NAME;\
  FIND_ELEMENT_IN_MOCKS_MAP1(_name_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    if (find_element_in_mocks_map_##_name_<this_type, result_type, arg1_type>(this, arg1) == true ||\
        find_element_in_mocks_map_##_name_<this_type, result_type, MockAnyValue>(this, AnyValue::_) == true) {\
      return;\
    }\
    SET_TEST_FAILED();\
    std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
    std::cerr << "with value: " << arg1 << std::endl << std::endl;\
  }\
  \
  EMPTY_MOCK1(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue)\
  MOCK_BODY1(_name_, function_traits<std::function<_type_>>::result_type, function_traits<std::function<_type_>>::arg<0>::type)\
  MOCK_BODY1(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue)

#define EMPTY_MOCK2(_name_, _result_type_, _arg1_type_, _arg2_type_)\
  void _name_(_arg1_type_ arg1, _arg2_type_ arg2) {}

#define FIND_ELEMENT_IN_MOCKS_MAP2(_name_)\
template <typename OT, typename RT, typename AT1, typename AT2>\
bool find_element_in_mocks_map_##_name_(OT* object, AT1 arg1, AT2 arg2) {\
  auto funptr = static_cast<RT(OT::*)(AT1, AT2)>(&OT::_name_);\
  auto iter = g_map<OT, AT1, AT2>.find(object);\
  if (iter == g_map<OT, AT1, AT2>.end()) {\
    return false;\
  }\
  auto elem = std::find_if(std::begin(iter->second),\
                           std::end(iter->second),\
                           [funptr, arg1, arg2](const auto& tuple) -> bool {\
                             return std::get<1>(tuple) == funptr &&\
                                    std::get<2>(tuple) == arg1 &&\
                                    std::get<3>(tuple) == arg2;\
                           });\
  if (elem == iter->second.end()) {\
    return false;\
  }\
  iter->second.erase(elem);\
  return true;\
}

#define MOCK_BODY2(_name_, _result_type_, _arg1_type_, _arg2_type_)\
  void MOCK_##_name_(_arg1_type_ arg1, _arg2_type_ arg2) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    auto funptr = static_cast<_result_type_(this_type::*)(_arg1_type_, _arg2_type_)>(&this_type::_name_);\
    auto iter = g_map<this_type, _arg1_type_, _arg2_type_>.find(this);\
    if (iter == g_map<this_type, _arg1_type_, _arg2_type_>.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr), _arg1_type_, _arg2_type_>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2));\
      g_map<this_type, _arg1_type_, _arg2_type_>[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2));\
    }\
  }

#define MOCK_METHOD2(_name_, _type_)\
  MockMapChecker<CLASS_TYPE,\
                 function_traits<std::function<_type_>>::arg<0>::type,\
                 function_traits<std::function<_type_>>::arg<1>::type> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 MockAnyValue,\
                 function_traits<std::function<_type_>>::arg<1>::type> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 function_traits<std::function<_type_>>::arg<0>::type,\
                 MockAnyValue> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 MockAnyValue,\
                 MockAnyValue> UNIQUE_NAME;\
  FIND_ELEMENT_IN_MOCKS_MAP2(_name_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using arg2_type = decltype(arg2);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    if (find_element_in_mocks_map_##_name_<this_type, result_type, arg1_type, arg2_type>(this, arg1, arg2) == true ||\
        find_element_in_mocks_map_##_name_<this_type, result_type, arg1_type, MockAnyValue>(this, arg1, AnyValue::_) == true ||\
        find_element_in_mocks_map_##_name_<this_type, result_type, MockAnyValue, arg2_type>(this, AnyValue::_, arg2) == true ||\
        find_element_in_mocks_map_##_name_<this_type, result_type, MockAnyValue, MockAnyValue>(this, AnyValue::_, AnyValue::_) == true) {\
      return;\
    }\
    SET_TEST_FAILED();\
    std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
    std::cerr << "with values: " << arg1 << " ; " << arg2 << std::endl << std::endl;\
  }\
  \
  EMPTY_MOCK2(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue, function_traits<std::function<_type_>>::arg<1>::type)\
  EMPTY_MOCK2(_name_, function_traits<std::function<_type_>>::result_type, function_traits<std::function<_type_>>::arg<0>::type, MockAnyValue)\
  EMPTY_MOCK2(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue, MockAnyValue)\
  MOCK_BODY2(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            function_traits<std::function<_type_>>::arg<0>::type,\
            function_traits<std::function<_type_>>::arg<1>::type)\
  MOCK_BODY2(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            MockAnyValue,\
            function_traits<std::function<_type_>>::arg<1>::type)\
  MOCK_BODY2(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            function_traits<std::function<_type_>>::arg<0>::type,\
            MockAnyValue)\
  MOCK_BODY2(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            MockAnyValue,\
            MockAnyValue)

#define EMPTY_MOCK3(_name_, _result_type_, _arg1_type_, _arg2_type_, _arg3_type_)\
  void _name_(_arg1_type_ arg1, _arg2_type_ arg2, _arg3_type_ arg3) {}

#define FIND_ELEMENT_IN_MOCKS_MAP3(_name_)\
template <typename OT, typename RT, typename AT1, typename AT2, typename AT3>\
bool find_element_in_mocks_map(OT* object, AT1 arg1, AT2 arg2, AT3 arg3) {\
  auto funptr = static_cast<RT(OT::*)(AT1, AT2, AT3)>(&OT::_name_);\
  auto iter = g_map<OT, AT1, AT2, AT3>.find(object);\
  if (iter == g_map<OT, AT1, AT2, AT3>.end()) {\
    return false;\
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
    return false;\
  }\
  iter->second.erase(elem);\
  return true;\
}

#define MOCK_BODY3(_name_, _result_type_, _arg1_type_, _arg2_type_, _arg3_type_)\
  void MOCK_##_name_(_arg1_type_ arg1, _arg2_type_ arg2, _arg3_type_ arg3) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    auto funptr = static_cast<_result_type_(this_type::*)(_arg1_type_, _arg2_type_, _arg3_type_)>(&this_type::_name_);\
    auto iter = g_map<this_type, _arg1_type_, _arg2_type_, _arg3_type_>.find(this);\
    if (iter == g_map<this_type, _arg1_type_, _arg2_type_, _arg3_type_>.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr), _arg1_type_, _arg2_type_, _arg3_type_>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2, arg3));\
      g_map<this_type, _arg1_type_, _arg2_type_, _arg3_type_>[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2, arg3));\
    }\
  }

#define MOCK_METHOD3(_name_, _type_)\
  MockMapChecker<CLASS_TYPE,\
                 function_traits<std::function<_type_>>::arg<0>::type,\
                 function_traits<std::function<_type_>>::arg<1>::type,\
                 function_traits<std::function<_type_>>::arg<2>::type> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 MockAnyValue,\
                 function_traits<std::function<_type_>>::arg<1>::type,\
                 function_traits<std::function<_type_>>::arg<2>::type> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 function_traits<std::function<_type_>>::arg<0>::type,\
                 MockAnyValue,\
                 function_traits<std::function<_type_>>::arg<2>::type> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 function_traits<std::function<_type_>>::arg<0>::type,\
                 function_traits<std::function<_type_>>::arg<1>::type,\
                 MockAnyValue> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 MockAnyValue,\
                 MockAnyValue,\
                 function_traits<std::function<_type_>>::arg<2>::type> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 MockAnyValue,\
                 function_traits<std::function<_type_>>::arg<1>::type,\
                 MockAnyValue> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 function_traits<std::function<_type_>>::arg<0>::type,\
                 MockAnyValue,\
                 MockAnyValue> UNIQUE_NAME;\
  MockMapChecker<CLASS_TYPE,\
                 MockAnyValue,\
                 MockAnyValue,\
                 MockAnyValue> UNIQUE_NAME;\
  FIND_ELEMENT_IN_MOCKS_MAP3(_name_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2,\
      function_traits<std::function<_type_>>::arg<2>::type arg3) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using arg2_type = decltype(arg2);\
    using arg3_type = decltype(arg3);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    if (find_element_in_mocks_map<this_type, result_type, arg1_type, arg2_type, arg3_type>(this, arg1, arg2, arg3) == true ||\
        find_element_in_mocks_map<this_type, result_type, arg1_type, arg2_type, MockAnyValue>(this, arg1, arg2, AnyValue::_) == true ||\
        find_element_in_mocks_map<this_type, result_type, arg1_type, MockAnyValue, arg3_type>(this, arg1, AnyValue::_, arg3) == true ||\
        find_element_in_mocks_map<this_type, result_type, MockAnyValue, arg2_type, arg3_type>(this, AnyValue::_, arg2, arg3) == true ||\
        find_element_in_mocks_map<this_type, result_type, arg1_type, MockAnyValue, MockAnyValue>(this, arg1, AnyValue::_, AnyValue::_) == true ||\
        find_element_in_mocks_map<this_type, result_type, MockAnyValue, arg2_type, MockAnyValue>(this, AnyValue::_, arg2, AnyValue::_) == true ||\
        find_element_in_mocks_map<this_type, result_type, MockAnyValue, MockAnyValue, arg3_type>(this, AnyValue::_, AnyValue::_, arg3) == true ||\
        find_element_in_mocks_map<this_type, result_type, MockAnyValue, MockAnyValue, MockAnyValue>(this, AnyValue::_, AnyValue::_, AnyValue::_) == true) {\
      return;\
    }\
    SET_TEST_FAILED();\
    std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
    std::cerr << "with values: " << arg1 << " ; " << arg2 << " ; " << arg3 << std::endl << std::endl;\
  }\
  \
  EMPTY_MOCK3(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue, function_traits<std::function<_type_>>::arg<1>::type, function_traits<std::function<_type_>>::arg<2>::type)\
  EMPTY_MOCK3(_name_, function_traits<std::function<_type_>>::result_type, function_traits<std::function<_type_>>::arg<0>::type, MockAnyValue, function_traits<std::function<_type_>>::arg<2>::type)\
  EMPTY_MOCK3(_name_, function_traits<std::function<_type_>>::result_type, function_traits<std::function<_type_>>::arg<0>::type, function_traits<std::function<_type_>>::arg<1>::type, MockAnyValue)\
  EMPTY_MOCK3(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue, MockAnyValue, function_traits<std::function<_type_>>::arg<2>::type)\
  EMPTY_MOCK3(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue, function_traits<std::function<_type_>>::arg<1>::type, MockAnyValue)\
  EMPTY_MOCK3(_name_, function_traits<std::function<_type_>>::result_type, function_traits<std::function<_type_>>::arg<0>::type, MockAnyValue, MockAnyValue)\
  EMPTY_MOCK3(_name_, function_traits<std::function<_type_>>::result_type, MockAnyValue, MockAnyValue, MockAnyValue)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            function_traits<std::function<_type_>>::arg<0>::type,\
            function_traits<std::function<_type_>>::arg<1>::type,\
            function_traits<std::function<_type_>>::arg<2>::type)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            MockAnyValue,\
            function_traits<std::function<_type_>>::arg<1>::type,\
            function_traits<std::function<_type_>>::arg<2>::type)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            function_traits<std::function<_type_>>::arg<0>::type,\
            MockAnyValue,\
            function_traits<std::function<_type_>>::arg<2>::type)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            function_traits<std::function<_type_>>::arg<0>::type,\
            function_traits<std::function<_type_>>::arg<1>::type,\
            MockAnyValue)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            MockAnyValue,\
            MockAnyValue,\
            function_traits<std::function<_type_>>::arg<2>::type)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            MockAnyValue,\
            function_traits<std::function<_type_>>::arg<1>::type,\
            MockAnyValue)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            function_traits<std::function<_type_>>::arg<0>::type,\
            MockAnyValue,\
            MockAnyValue)\
  MOCK_BODY3(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            MockAnyValue,\
            MockAnyValue,\
            MockAnyValue)

#define FIND_ELEMENT_IN_MOCKS_MAP4(_name_)\
template <typename OT, typename RT, typename AT1, typename AT2, typename AT3, typename AT4>\
bool find_element_in_mocks_map(OT* object, AT1 arg1, AT2 arg2, AT3 arg3, AT4 arg4) {\
  auto funptr = static_cast<RT(OT::*)(AT1, AT2, AT3, AT4)>(&OT::_name_);\
  auto iter = g_map<OT, AT1, AT2, AT3, AT4>.find(object);\
  if (iter == g_map<OT, AT1, AT2, AT3, AT4>.end()) {\
    return false;\
  }\
  auto elem = std::find_if(std::begin(iter->second),\
                           std::end(iter->second),\
                           [funptr, arg1, arg2, arg3, arg4](const auto& tuple) -> bool {\
                             return std::get<1>(tuple) == funptr &&\
                                    std::get<2>(tuple) == arg1 &&\
                                    std::get<3>(tuple) == arg2 &&\
                                    std::get<4>(tuple) == arg3 &&\
                                    std::get<5>(tuple) == arg4;\
                           });\
  if (elem == iter->second.end()) {\
    return false;\
  }\
  iter->second.erase(elem);\
  return true;\
}

#define MOCK_BODY4(_name_, _result_type_, _arg1_type_, _arg2_type_, _arg3_type_, _arg4_type_)\
  void MOCK_##_name_(_arg1_type_ arg1, _arg2_type_ arg2, _arg3_type_ arg3, _arg4_type_ arg4) {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    auto funptr = static_cast<_result_type_(this_type::*)(_arg1_type_, _arg2_type_, _arg3_type_, _arg4_type_)>(&this_type::_name_);\
    auto iter = g_map<this_type, _arg1_type_, _arg2_type_, _arg3_type_, _arg4_type_>.find(this);\
    if (iter == g_map<this_type, _arg1_type_, _arg2_type_, _arg3_type_, _arg4_type_>.end()) {\
      std::vector<std::tuple<std::string, decltype(funptr), _arg1_type_, _arg2_type_, _arg3_type_, _arg4_type_>> vec;\
      vec.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2, arg3, arg4));\
      g_map<this_type, _arg1_type_, _arg2_type_, _arg3_type_, _arg4_type_>[this] = vec;\
    } else {\
      iter->second.push_back(std::make_tuple(getNameForType<this_type>() + "::" + #_name_, funptr, arg1, arg2, arg3, arg4));\
    }\
  }

#define MOCK_METHOD4(_name_, _type_)\
  MockMapChecker<CLASS_TYPE,\
                 function_traits<std::function<_type_>>::arg<0>::type,\
                 function_traits<std::function<_type_>>::arg<1>::type,\
                 function_traits<std::function<_type_>>::arg<2>::type,\
                 function_traits<std::function<_type_>>::arg<3>::type> UNIQUE_NAME;\
  FIND_ELEMENT_IN_MOCKS_MAP4(_name_)\
  function_traits<std::function<_type_>>::result_type _name_(\
      function_traits<std::function<_type_>>::arg<0>::type arg1,\
      function_traits<std::function<_type_>>::arg<1>::type arg2,\
      function_traits<std::function<_type_>>::arg<2>::type arg3,\
      function_traits<std::function<_type_>>::arg<3>::type arg4) override {\
    using this_type = std::remove_reference<decltype(*this)>::type;\
    using arg1_type = decltype(arg1);\
    using arg2_type = decltype(arg2);\
    using arg3_type = decltype(arg3);\
    using arg4_type = decltype(arg4);\
    using result_type = function_traits<std::function<_type_>>::result_type;\
    if (find_element_in_mocks_map<this_type, result_type, arg1_type, arg2_type, arg3_type, arg4_type>(this, arg1, arg2, arg3, arg4) == true) {\
      return;\
    }\
    SET_TEST_FAILED();\
    std::cerr << "Not expected mock function called:" << getNameForType<this_type>() << "::" << #_name_ << std::endl;\
    std::cerr << "with values: " << arg1 << " ; " << arg2 << " ; " << arg3 << " ; " << arg4 << std::endl << std::endl;\
  }\
  MOCK_BODY4(_name_,\
            function_traits<std::function<_type_>>::result_type,\
            function_traits<std::function<_type_>>::arg<0>::type,\
            function_traits<std::function<_type_>>::arg<1>::type,\
            function_traits<std::function<_type_>>::arg<2>::type,\
            function_traits<std::function<_type_>>::arg<3>::type)

#define EXPECT_CALL(obj, fun) obj.MOCK_##fun
#endif  // MOCK_H
