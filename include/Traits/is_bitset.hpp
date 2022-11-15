#pragma once
#include <type_traits>
#include <bitset>

template<typename T>
struct is_bitset : std::false_type {};

template<std::size_t N>
struct is_bitset<std::bitset<N>> : std::true_type {};