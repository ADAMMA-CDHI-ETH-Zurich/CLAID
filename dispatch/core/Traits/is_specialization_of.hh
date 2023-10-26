#pragma once
#include <type_traits>
// Can be used to check whether a given type is a template 
// specialization of a (generic / templated) class.


// E.g.:
// is_specialization_of<std::vector<int>, std::vector> // true, because is std::vector<int> is a specialization of std::vector.
// is_specialization_of<std::string, std::vector>      // false, because string has nothing to do with std::vector.
template< class T, template<class...> class Primary >
struct is_specialization_of : std::false_type
{

};

template< template<class...> class Primary, class... Args >
struct is_specialization_of< Primary<Args...>, Primary> : std::true_type
{
    
};