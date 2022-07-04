#pragma once
#include <string>

// Even though we treat std::shared_ptr as a built-in type in RecursiveReflector, this reflect function is still needed.
// The else case in the is_class case of ReflectorType is still evaluated at compile time for string.
// If we didnt have a reflect function, we would get an error.
template<typename Reflector, typename T>
void reflect(Reflector& r, std::shared_ptr<T>& string)
{
  
}