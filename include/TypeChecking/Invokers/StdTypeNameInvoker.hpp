#pragma once

#include "Traits/is_specialization_of.hpp"
#include "Traits/is_bitset.hpp"
#include "TypeChecking/Invokers/TypeNameInvoker.hpp"
#include "TypeChecking/Helpers/Helpers.hpp"
#include <set>
#include <map>
#include <deque>
#include <queue>
#include <list>
#include <stack>
#include <bitset>
namespace claid
{
    namespace TypeChecking
    {
        template<typename Type>
            struct ____INVALID_TYPE_OF_NAMESPACE_STD_IN_GETTING_TYPE_NAME_{
            static_assert(sizeof(Type)==0,
                "Cannot get platform independent type name of the type mentioned above."
                "The type is part of namespace \"std::\", therefore part of the standard library."
                "For the standard library, it is not guaranteed that type names are equal across different compilers, "
                "as they might have been implemented in different namespaces (e.g. std::basic_string in clang vs std::__cxx11::basic_string in gcc."
                "Therefore, type names have been manually assigned to types in the std:: namespace. If you see this error, then probably you used a type of "
                "the standard library, that we forgot to include in type name generation. Please see file StdTypeNameInvoker.hpp.");
            static void invoke() {}
        };

        template<typename T, class Enable = void>
        struct StdTypeNameInvoker 
        {
            static std::string call() 
            {
                // Only types of standard library we explicitly covered are safe.
                ____INVALID_TYPE_OF_NAMESPACE_STD_IN_GETTING_TYPE_NAME_<T>::invoke();
                return "invalid type";
            }           

        }; 


        // Pointer

        template<class T>
        struct StdTypeNameInvoker<T, typename std::enable_if<std::is_pointer<T>::value>::type> 
        {
            static std::string call() 
            {
                return TypeNameInvoker<typename Helpers::getDereferencedType<T>::type>::call() + std::string("*");
            }
        };

        
        // CONTAINERS SINGLE TYPE
        #define STD_TYPENAME_CONTAINER_SINGLE(name)\
        template<class T> \
        struct StdTypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, name>::value>::type> \
        {\
            static std::string call() \
            {\
                return std::string(#name) + std::string("<") + TypeNameInvoker<typename T::value_type>::call() + std::string(">");\
            }\
        };\

        STD_TYPENAME_CONTAINER_SINGLE(std::vector)
        STD_TYPENAME_CONTAINER_SINGLE(std::set)
        STD_TYPENAME_CONTAINER_SINGLE(std::multiset)
        STD_TYPENAME_CONTAINER_SINGLE(std::deque)
        STD_TYPENAME_CONTAINER_SINGLE(std::queue)
        STD_TYPENAME_CONTAINER_SINGLE(std::list)
        STD_TYPENAME_CONTAINER_SINGLE(std::stack)

        // CONTAINERS MULTIPLE TYPE

        template<class T>
        struct StdTypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, std::map>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::map<") + TypeNameInvoker<typename T::key_type>::call() + std::string(",") + TypeNameInvoker<typename T::mapped_type>::call() + std::string(">");
            }
        };

        template<class T>
        struct StdTypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, std::multimap>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::multimap<") + TypeNameInvoker<typename T::key_type>::call() + std::string(",") + TypeNameInvoker<typename T::mapped_type>::call() + std::string(">");
            }
        };

        template<class T>
        struct StdTypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, std::pair>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::pair<") + TypeNameInvoker<typename T::first_type>::call() + std::string(",") + TypeNameInvoker<typename T::second_type>::call() + std::string(">");
            }
        };

       

        

        template<class T>
        struct StdTypeNameInvoker<T, typename std::enable_if<is_bitset<T>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::bitset<") + std::to_string(Helpers::getBitsetSize<T>::Length) + std::string(">");
            }
        };

        template<class T>
        struct StdTypeNameInvoker<T, typename std::enable_if<std::is_same<T, std::string>::value>::type> 
        {
            static std::string call() 
            {
                return "std::string";
            }
        };

        template<class T>
        struct StdTypeNameInvoker<T, typename std::enable_if<std::is_same<T, std::wstring>::value>::type> 
        {
            static std::string call() 
            {
                return "std::wstring";
            }
        };


    }
}