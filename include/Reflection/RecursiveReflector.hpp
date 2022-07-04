#pragma once
#include "AbstractReflector.hpp"
#include <string>
#include <type_traits>
#include "ClassInvoker.hpp"

namespace portaible
{
    template<typename T> struct is_integer_no_bool : public std::false_type {};

    #define ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(type) \
    template<>\
    struct is_integer_no_bool<type> : public std::true_type {};

    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(short)
    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(int)
    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(long)
    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(long long)
    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(unsigned int)
    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(unsigned long)
    ADD_TO_IS_INTEGER_NO_BOOL_TYPE_TRAIT(unsigned long long)
  
  

    template<typename Derived>
    class RecursiveReflector : public AbstractReflector
    {
        

        template<typename Type>
            struct ____INVALID_TYPE_IN_REFLECTION_TYPE_{
            static_assert(sizeof(Type)==0,
                "The Type (see above) is not supported for reflection.");
            static void invoke() {}
        };

        protected:
            void* currentDefaultValue = nullptr;

            template<typename T>
            T& getCurrentDefaultValue()
            {
                return *static_cast<T*>(this->currentDefaultValue);
            }

            bool defaultValueCurrentlySet()
            {
                return currentDefaultValue != nullptr;
            }

        template<typename T, class Enable = void>
        struct ReflectorType 
        {
            static void call(const char* property, Derived& r, T& member) 
            {
                ____INVALID_TYPE_IN_REFLECTION_TYPE_<T>::invoke();
            }

        }; 
        
        // ATOMIC TYPES
            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_floating_point<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, T& member) 
                {
                    r.callFloatOrDouble(property, member);
                }
            };

            template<class T>
            struct ReflectorType<T, typename std::enable_if<is_integer_no_bool<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, T& member) 
                {

                    r.callInt(property, member);
                
                }
            };

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_same<T, bool>::value>::type> 
            {
                static void call(const char* property, Derived& r, T& member) 
                {
                    r.callBool(property, member);
                }
            };
        // CLASS TYPES

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_class<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, T& member) 
                {
                    ClassInvoker<Derived, T>::call(r, property, member);
                }
            };

       
        // POINTER TYPES

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_pointer<T>::value>::type>
            {
                static void call(const char* property, Derived& r, T& member)
                {
                    r.callPointer(property, member);
                }
            };

            

        public:
            virtual void beginSequence()
            {

            }

            virtual void endSequence()
            {
                
            }

            template<typename T>
            void invokeReflectOnObject(T& obj)
            {
                ReflectorInvoker<Derived, T>::call(*This(), obj);
            }

            template<typename T>
            void property(const char* property, T& member, const char* comment)
            {
                this->currentDefaultValue = nullptr;
                ReflectorType<T>::call(property, *this->This(), member);
            }

            template<typename T>
            void property(const char* property, T& member, const char* comment, T defaultValue)
            {
                this->currentDefaultValue = &defaultValue;
                ReflectorType<T>::call(property, *this->This(), member);
            }

            template<typename T>
            void member(const char* property, T& member, const char* comment)
            {
                this->currentDefaultValue = nullptr;
                ReflectorType<T>::call(property, *this->This(), member);
            }

            template<typename T>
            void member(const char* property, T& member, const char* comment, T defaultValue)
            {
                this->currentDefaultValue = &defaultValue;
                ReflectorType<T>::call(property, *this->This(), member);
            }

            template<typename T>
            void callOnObject(const char* name, T& obj)
            {
                ReflectorType<T>::call(name, *this->This(), obj);
            }

            Derived* This()
            {
                return static_cast<Derived*>(this);
            }

    };  
}