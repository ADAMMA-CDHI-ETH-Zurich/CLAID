#pragma once
#include <type_traits>
#include <memory.h>
#include <iostream>
#include "ReflectedVariable.hpp"
namespace claid
{
    // For any other class 
    template<typename Reflector, typename T, class Enable = void>
    struct ClassInvoker 
    {
        // default case
        static void call(Reflector& r, const char* property, ReflectedVariable<T> member) 
        {
            // Any other class
            r.callBeginClass(property, member);
            // Call reflect method on that class recursively.

            r.forwardReflectedVariable(member);
            r.callEndClass(property, member);
        }

    }; 

   // For class string
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<std::is_same<T, std::string>::value>::type>
    {
        static void call(Reflector& r, const char* property, ReflectedVariable<T> member) 
        {
            r.callString(property, member);
        }

    }; 

    // For class std::shared_ptr
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<std::is_same<T, std::shared_ptr<typename T::element_type>>::value>::type>
    {
        static void call(Reflector& r, const char* property, ReflectedVariable<T> member) 
        {
            r.callBeginClass(property, member);
            r.callSharedPointer(property, member);
            r.callEndClass(property, member);
        }

    };

}