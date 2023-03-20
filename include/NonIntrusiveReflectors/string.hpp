#pragma once
#include <string>
#include "Reflection/ReflectedVariable.hpp"

// Even though we treat std::string as a built-in type in RecursiveReflector, this reflect function is still needed.
// The else case in the is_class case of ReflectorType is still evaluated at compile time for string.
// If we didnt have a reflect function, we would get an error.
template<typename Reflector>
void reflect(Reflector& r, std::string& string)
{
    claid::ReflectedVariable<std::string> var = claid::make_reflected_variable("", string);
    r.callString("", var);
}