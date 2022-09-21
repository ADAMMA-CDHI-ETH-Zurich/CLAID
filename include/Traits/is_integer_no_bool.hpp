#pragma once

template<typename T> struct is_integer_no_bool : public std::false_type {};

#define ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(type) \
template<>\
struct is_integer_no_bool<type> : public std::true_type {};


#ifndef byte
    #include "Utilities/byte.hpp"
    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(byte)
#endif

ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(short)
ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(int)
ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(long)
ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(long long)
ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(unsigned int)
ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(unsigned long)
ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(unsigned long long)