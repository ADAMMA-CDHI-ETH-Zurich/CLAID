#pragma once

namespace claid
{
    template<typename T> struct is_integer_no_bool_no_char : public std::false_type {};
}

#define CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(type) \
namespace claid\
{\
    template<>\
    struct is_integer_no_bool_no_char<type> : public std::true_type {};\
}

CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(signed short)
CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(unsigned short)

CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(signed int)
CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(unsigned int)

CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(signed long)
CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(unsigned long)

CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(signed long long)
CLAID_IS_INTEGER_NO_BOOL_NO_CHAR(unsigned long long)


