#pragma once
#include <utility>
#include <type_traits>


#define HAS_NON_MEMBER_FUNCTION_WITH_ZERO_ARGUMENTS_DEFINITION_DETECTOR(Identifier, ReturnType, FunctionName)   \
template <typename = ReturnType>                                                     \
struct has_non_member_function_##Identifier                                             \
{                                                                                    \
    typedef std::false_type type;                                                    \
    enum { value = false };                                                          \
};                                                                                   \
                                                                                     \
template <>                                                                          \
struct has_non_member_function_##Identifier<decltype(FunctionName())>                   \
{                                                                                    \
    typedef std::true_type type;                                                     \
    enum { value = true };                                                           \
};

#define HAS_NON_MEMBER_FUNCTION_WITH_ZERO_ARGUMENTS_DETECTOR(ReturnType, FunctionName)                  \
        HAS_NON_MEMBER_FUNCTION_WITH_ZERO_ARGUMENTS_DEFINITION_DETECTOR(FunctionName, ReturnType, FunctionName)

//

#define HAS_NON_MEMBER_FUNCTION_WITH_ONE_ARGUMENT_DEFINITION_DETECTOR(Identifier, ReturnType, FunctionName)   \
template <typename P0, typename = ReturnType>                                        \
struct has_non_member_function_##Identifier                                             \
{                                                                                    \
    typedef std::false_type type;                                                    \
    enum { value = false };                                                          \
};                                                                                   \
                                                                                     \
template <typename P0>                                                               \
struct has_non_member_function_##Identifier<P0,                                         \
                                         decltype(FunctionName(std::declval<P0>()))> \
{                                                                                    \
    typedef std::true_type type;                                                     \
    enum { value = true };                                                           \
};
#define HAS_NON_MEMBER_FUNCTION_WITH_ONE_ARGUMENT_DETECTOR(ReturnType, FunctionName)                  \
        HAS_NON_MEMBER_FUNCTION_WITH_ONE_ARGUMENT_DEFINITION_DETECTOR(FunctionName, ReturnType, FunctionName)

//

#define HAS_NON_MEMBER_FUNCTION_WITH_TWO_ARGUMENTS_DEFINITION_DETECTOR(Identifier, ReturnType, FunctionName)   \
template <typename P0, typename P1, typename = ReturnType>                           \
struct has_non_member_function_##Identifier                                             \
{                                                                                    \
    typedef std::false_type type;                                                    \
    enum { value = false };                                                          \
};                                                                                   \
                                                                                     \
template <typename P0, typename P1>                                                  \
struct has_non_member_function_##Identifier<P0, P1,                                     \
                                         decltype(FunctionName(std::declval<P0>(),   \
                                                               std::declval<P1>()))> \
{                                                                                    \
    typedef std::true_type type;                                                     \
    enum { value = true };                                                           \
};

#define HAS_NON_MEMBER_FUNCTION_WITH_TWO_ARGUMENTS_DETECTOR(ReturnType, FunctionName)\
        HAS_NON_MEMBER_FUNCTION_WITH_TWO_ARGUMENTS_DEFINITION_DETECTOR(FunctionName, ReturnType, FunctionName)