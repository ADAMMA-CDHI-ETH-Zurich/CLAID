#pragma once
#include <type_traits>
#include <memory.h>
#include <iostream>
#include "Traits/is_specialization_of.hpp"
#include "Reflection/VariableWithGetterSetter.hpp"

#include "AbstractReflector.hpp"

namespace claid
{
    template<typename Type>
        struct ____MISSING_REFLECT_METHOD_FOR_{
        static_assert(sizeof(Type)==0,
            "Cannot find an (unambiguous) intrusive or nonintrusive reflect method for Type. "
            "If Type is a class that you have implemented, then you should "
            "implement its reflect() method.");
        static void invoke() {}
    };

    // ClassReflectFunctionInvoker is used to invoke the reflect function of classes that have either internal or external 
    // reflect function directly on an object, that is no member of another object.
    template<typename Reflector, typename T, class Enable = void>
    struct ClassReflectFunctionInvoker
    {
        // default case
        static void call(Reflector& r, T& member) 
        {
            ____MISSING_REFLECT_METHOD_FOR_<T>::invoke();
        }
    };

    template<typename Reflector, typename T>
    struct ClassReflectFunctionInvoker<Reflector, T, typename std::enable_if<has_mem_reflect<T>::value>::type>
    {
        // default case
        static void call(Reflector& r, T& member) 
        {
            member.reflect(r);
        }
    };

    template<typename Reflector, typename T>
    struct ClassReflectFunctionInvoker<Reflector, T, typename std::enable_if<has_non_member_function_reflect<Reflector&, T&>::value>::type>
    {
        // default case
        static void call(Reflector& r, T& member) 
        {
            reflect(r, member);
        }
    };

    // ClassInvoker is used to invoke the either internal or external reflect function of a class,
    // when the object itself is a member of another class.
    template<typename Reflector, typename T, class Enable = void>
    struct ClassInvoker 
    {
        // default case
        static void call(Reflector& r, const char* property, T& member) 
        {
            ____MISSING_REFLECT_METHOD_FOR_<T>::invoke();
        }

    }; 

    // For classes with internal reflect function
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<has_mem_reflect<T>::value>::type>
    {
        // default case
        static void call(Reflector& r, const char* property, T& member) 
        {
            // Any other class
            r.callBeginClass(property, member);
            // Call reflect method on that class recursively.

            member.reflect(r);
            //r.invokeReflectOnObject(member, false);
            r.callEndClass(property, member);
        }

    }; 

    // For classes with external reflect function
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<has_non_member_function_reflect<Reflector&, T&>::value && !std::is_same<T, std::string>::value && !is_specialization_of<T, std::shared_ptr>::value>::type>
    {
        // default case
        static void call(Reflector& r, const char* property, T& member) 
        {
            // Any other class
            r.callBeginClass(property, member);
            // Call reflect method on that class recursively.

            reflect(r, member);
            //r.invokeReflectOnObject(member, false);
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

    // // For class "VariableWithGetterSetter"
    // template<typename Reflector, typename T>
    // struct ClassInvoker<Reflector, T, typename std::enable_if<is_specialization_of<T, claid::VariableWithGetterSetter>::value>::type>
    // {
    //     static void call(Reflector& r, const char* property, T& member) 
    //     {
    //         member.reflect(r);
    //     }

    // }; 

    // For class std::shared_ptr
    template<typename Reflector, typename T>
    struct ClassInvoker<Reflector, T, typename std::enable_if<is_specialization_of<T, std::shared_ptr>::value>::type>
    {
        static void call(Reflector& r, const char* property, T& member) 
        {
            r.callBeginClass(property, member);
            r.callSharedPointer(property, member);
            r.callEndClass(property, member);
        }

    };

}