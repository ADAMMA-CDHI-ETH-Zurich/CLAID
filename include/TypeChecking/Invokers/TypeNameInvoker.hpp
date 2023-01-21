#pragma once
#include <type_traits>
#include "Traits/is_specialization_of.hpp"
#include "TypeChecking/CompileTimeTypeNameDemangling.hpp"
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

        // If type is const, we remove constness.
        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<std::is_const<T>::value>::type> 
        {
            static std::string call() 
            {
                return TypeNameInvoker<typename std::remove_const<T>::type>::call();
            }
        };    

        // Pointer

        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<std::is_pointer<T>::value>::type> 
        {
            static std::string call() 
            {
                return TypeNameInvoker<typename Helpers::getDereferencedType<T>::type>::call() + std::string("*");
            }
        };

        
        // CONTAINERS SINGLE TYPE
        #define STD_TYPENAME_CONTAINER_SINGLE(name)\
        template<class T> \
        struct TypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, name>::value>::type> \
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
        struct TypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, std::map>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::map<") + TypeNameInvoker<typename T::key_type>::call() + std::string(",") + TypeNameInvoker<typename T::mapped_type>::call() + std::string(">");
            }
        };

        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, std::multimap>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::multimap<") + TypeNameInvoker<typename T::key_type>::call() + std::string(",") + TypeNameInvoker<typename T::mapped_type>::call() + std::string(">");
            }
        };

        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<is_specialization_of<T, std::pair>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::pair<") + TypeNameInvoker<typename T::first_type>::call() + std::string(",") + TypeNameInvoker<typename T::second_type>::call() + std::string(">");
            }
        };


        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<is_bitset<T>::value>::type> 
        {
            static std::string call() 
            {
                return std::string("std::bitset<") + std::to_string(Helpers::getBitsetSize<T>::Length) + std::string(">");
            }
        };

        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<std::is_same<T, std::string>::value>::type> 
        {
            static std::string call() 
            {
                return "std::string";
            }
        };

        template<class T>
        struct TypeNameInvoker<T, typename std::enable_if<std::is_same<T, std::wstring>::value>::type> 
        {
            static std::string call() 
            {
                return "std::wstring";
            }
        }; 
    }
}