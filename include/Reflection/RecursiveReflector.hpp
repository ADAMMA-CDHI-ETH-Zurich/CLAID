#pragma once
#include "AbstractReflector.hpp"
#include <string>
#include <type_traits>
#include "ClassInvoker.hpp"
#include "Traits/is_integer_no_bool_no_char.hpp"
#include "ReflectedVariable.hpp"
namespace claid
{
    template<typename Derived>
    class RecursiveReflector : public AbstractReflector
    {
        // Has to be set by each specialization.
        // static std::string REFLECTOR_NAME;

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
            static void call(const char* property, Derived& r, ReflectedVariable<T> member) 
            {
                ____INVALID_TYPE_IN_REFLECTION_TYPE_<T>::invoke();
            }

        }; 

        // CONST (we cast it away, for now..)
            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_const<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member) 
                {
                    typedef typename std::remove_const<T>::type NonConstType; 
                    NonConstType& non_const_member = *const_cast<NonConstType*>(&member);



                    ReflectorType<NonConstType>::call(property, r, non_const_member);
                }
            };
        
        // ATOMIC TYPES
            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_floating_point<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member) 
                {
                    r.callFloatOrDouble(property, member);
                }
            };

            template<class T>
            struct ReflectorType<T, typename std::enable_if<is_integer_no_bool_no_char<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member) 
                {

                    r.callInt(property, member);
                
                }
            };

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_same<T, bool>::value>::type> 
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member) 
                {
                    r.callBool(property, member);
                }
            };

            // Why do we explicitly need to distinguish between signed and unsigned char?
            // Read the following: https://stackoverflow.com/questions/16503373/difference-between-char-and-signed-char-in-c
            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_same<T, signed char>::value || std::is_same<T, unsigned char>::value || 
                            std::is_same<T, char>::value || std::is_same<T, char16_t>::value || std::is_same<T, char32_t>::value >::type> 
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member) 
                {
                    r.callChar(property, member);
                }
            };

        // CLASS TYPES

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_class<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member) 
                {
                    ClassInvoker<Derived, T>::call(r, property, member);
                }
            };

       
        // POINTER TYPES

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_pointer<T>::value>::type>
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member)
                {
                    r.callPointer(property, member);
                }
            };

        // ENUM TYPES

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_enum<T>::value>::type>
            {
                static void call(const char* property, Derived& r, ReflectedVariable<T> member)
                {
                    r.callEnum(property, member);
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
            void forwardReflectedVariable(ReflectedVariable<T> member)
            {
                ReflectorInvoker<Derived, T>::call(*This(), member);
            }

            // Calls the reflect function of the given object.
            template<typename T>
            void invokeReflectOnObject(T& obj)
            {
                ReflectorInvoker<Derived, T>::call(*This(), obj);
            }

           
            template<typename T>
            void member(const char* name, T& mem, const char* comment)
            {
                this->currentDefaultValue = nullptr;
       
                auto variable = make_reflected_variable(name, mem);
                ReflectorType<typename decltype(variable)::ValueType>::call(name, *this->This(), variable);
            }

            template<typename T>
            void member(const char* name, T& mem, const char* comment, T defaultValue)
            {
                this->currentDefaultValue = &defaultValue;
          

                ReflectedVariable<T> variable = make_reflected_variable(name, mem);

                ReflectorType<T>::call(name, *this->This(), variable);
            }

            // template<typename T, typename Class>
            // void member(const char* name, const char* comment, void (Class::*setter)(T&), const T& (Class::*getter)(), Class* obj)
            // {

            // }

            // template<typename T, typename Class>
            // void member(const char* name, const char* comment, void (Class::*setter)(const T&), T (Class::*getter)(), Class* obj)
            // {

            // }

          

            Derived* This()
            {
                return static_cast<Derived*>(this);
            }

    };  
}

