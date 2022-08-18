#pragma once
#include <type_traits>
#include "Traits/is_specialization_of.hpp"
#include "TypeChecking/CompileTimeTypeNameDemangling.hpp"

namespace portaible
{
    namespace TypeChecking
    {
        template<typename T, class Enable = void>
        struct TypeNameInvoker 
        {
            static std::string call() 
            {
                // For now, we assume every type to be safe, for which we did not define an exception below (e.g. types in standard library / std:: namespace).
                // We assume, that only standard types are implemented differently across different compilers and platforms,
                // thus having different. (e.g. std::basic_string<char> in clang, std::__cxx11::basic_string<char> in gcc etc.).
                // All user defined types "should" be safe.
                return compileTimeTypeNameByUsingFunctionName<T>().toStdString();
            }

        }; 
    }
}

#include "TypeChecking/Invokers/StdTypeNameInvoker.hpp"
namespace portaible
{
    namespace TypeChecking
    {
        // If type is const, we remove constness.
        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<std::is_const<T>::value>::type> 
        {
            static std::string call() 
            {
                return TypeNameInvoker<typename std::remove_const<T>::type>::call();
            }
        };    

        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<compileTimeTypeNameByUsingFunctionName<T>().isInNamespace("std::")>::type> 
        {
            static std::string call() 
            {
                return StdTypeNameInvoker<T>::call();
            }
        };     
    }
}