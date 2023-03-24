#pragma once
#include <type_traits>
#include <memory.h>
#include <iostream>
#include "Traits/is_specialization_of.hpp"
#include "Reflection/VariableWithGetterSetter.hpp"
namespace claid
{
    // For any other class 
    template<typename Reflector, typename T, class Enable = void>
    struct ClassInvoker 
    {
        // default case
        static void call(Reflector& r, const char* property, T& member) 
        {
            // Any other class
            r.callBeginClass(property, member);
            // Call reflect method on that class recursively.

            r.invokeReflectOnObject(member, false);
            r.callEndClass(property, member);
        }

    }; 

   // For class string
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<std::is_same<T, std::string>::value>::type>
    {
        static void call(Reflector& r, const char* property, T& member) 
        {
            r.callString(property, member);
        }

    }; 

    // For class string
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<is_specialization_of<T, claid::VariableWithGetterSetter>::value>::type>
    {
        static void call(Reflector& r, const char* property, T& member) 
        {
            member.reflect(r);
        }

    }; 

    // For class std::shared_ptr
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<std::is_same<T, std::shared_ptr<typename T::element_type>>::value>::type>
    {
        static void call(Reflector& r, const char* property, T& member) 
        {
            r.callBeginClass(property, member);
            r.callSharedPointer(property, member);
            r.callEndClass(property, member);
        }

    };

}