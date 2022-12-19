#pragma once

namespace claid
{
    template<typename T> struct is_integer_no_bool : public std::false_type {};
}

#define CLAID_IS_INTEGER_NO_BOOL(type) \
namespace claid\
{\
    template<>\
    struct is_integer_no_bool<type> : public std::true_type {};\
}

CLAID_IS_INTEGER_NO_BOOL(unsigned char)
CLAID_IS_INTEGER_NO_BOOL(signed char)

CLAID_IS_INTEGER_NO_BOOL(signed short)
CLAID_IS_INTEGER_NO_BOOL(unsigned short)

CLAID_IS_INTEGER_NO_BOOL(signed int)
CLAID_IS_INTEGER_NO_BOOL(unsigned int)

CLAID_IS_INTEGER_NO_BOOL(signed long)
CLAID_IS_INTEGER_NO_BOOL(unsigned long)

CLAID_IS_INTEGER_NO_BOOL(signed long long)
CLAID_IS_INTEGER_NO_BOOL(unsigned long long)


