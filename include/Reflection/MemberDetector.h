#pragma once
#include <utility>
#include <type_traits>


#define HAS_MEM_DETECTOR(mem)                                                                                     \
                                                                                                     \
template < typename T >                                                                               \
struct has_mem_##mem                                                                                  \
{                                                                                                     \
  struct yes {};                                                                                     \
  struct no  {};                                                                                     \
                                                                                                     \
  struct ambiguate_seed { char mem; };                                                               \
  template < typename U > struct ambiguate : U, ambiguate_seed {};                                   \
                                                                                                     \
  template < typename U, typename = decltype(&U::mem) > static constexpr no  test(int);              \
  template < typename                                 > static constexpr yes test(...);              \
                                                                                                     \
  static bool constexpr value = std::is_same<decltype(test< ambiguate<T> >(0)),yes>::value ;         \
  typedef std::integral_constant<bool,value>    type;                                                \
};



