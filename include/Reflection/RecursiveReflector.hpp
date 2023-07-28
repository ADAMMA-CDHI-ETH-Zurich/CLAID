#pragma once
#include "AbstractReflector.hpp"
#include <string>
#include <type_traits>
#include "ClassInvoker.hpp"
#include "Traits/is_integer_no_bool_no_char.hpp"
#include "NonIntrusiveReflectors/all.hpp"
#include "TypeChecking/TypeCheckingFunctions.hpp"
namespace claid
{
    template<typename Derived>
    class RecursiveReflector : public AbstractReflector
    {
        int numInvocations = 0;

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

            template<typename T>
            typename std::enable_if<std::is_copy_assignable<T>::value && std::is_constructible<T, T&>::value>::type
            assignDefaultValue(T& value)
            {
                value = this->getCurrentDefaultValue<T>();
            }

            template<typename T>
            typename std::enable_if<!std::is_copy_assignable<T>::value || !std::is_constructible<T, T&>::value>::type
            assignDefaultValue(T& value)
            {
                std::string dataTypeName = TypeChecking::getCompilerSpecificRunTimeNameOfObject(value);
                CLAID_THROW(claid::Exception, "Reflector " << this->getReflectorName() << " cannot assign default value to variable of type \"" << dataTypeName << "\".\n"
                << "Type " << dataTypeName << " is not copy assignable, possibly the copy assignment operator is implicitly deleted.");
            }

        template<typename T, class Enable = void>
        struct ReflectorType 
        {
            static void call(const char* property, Derived& r, T& member) 
            {
                ____INVALID_TYPE_IN_REFLECTION_TYPE_<T>::invoke();
            }

        }; 

        // CONST (we cast it away, for now..)
            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_const<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, T& member) 
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
                static void call(const char* property, Derived& r, T& member) 
                {
                    r.callFloatOrDouble(property, member);
                }
            };

            template<class T>
            struct ReflectorType<T, typename std::enable_if<is_integer_no_bool_no_char<T>::value>::type> 
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

            // Why do we explicitly need to distinguish between signed and unsigned char?
            // Read the following: https://stackoverflow.com/questions/16503373/difference-between-char-and-signed-char-in-c
            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_same<T, signed char>::value || std::is_same<T, unsigned char>::value || 
                            std::is_same<T, char>::value || std::is_same<T, char16_t>::value || std::is_same<T, char32_t>::value >::type> 
            {
                static void call(const char* property, Derived& r, T& member) 
                {
                    r.callChar(property, member);
                }
            };

        // CLASS TYPES

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_class<T>::value>::type> 
            {
                static void call(const char* property, Derived& r, T& member) 
                {
                    std::string test = TypeChecking::getCompilerSpecificRunTimeNameOfObject(member);
                    std::string test2 = TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>();
                    Logger::printfln("CLASSINVOKER");
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

        // ENUM TYPES

            template<class T>
            struct ReflectorType<T, typename std::enable_if<std::is_enum<T>::value>::type>
            {
                static void call(const char* property, Derived& r, T& member)
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
            void chooseReflectionFunction(const char* property, T& member)
            {
                ReflectorType<T>::call(property, *this->This(), member);
            }

            template<typename T>
            void forwardReflectorOnClass(T& obj)
            {
                ClassReflectFunctionInvoker<Derived, T>::call(*This(), obj);
            }

            // Used by untyped reflector
            template<typename T>
            void invokeReflectorOnNonPrimitiveType(T& obj, bool externalInvocation = true)
            {
                if(numInvocations == 0 && externalInvocation)
                {
                    this->This()->onInvocationStart(obj);
                }
                numInvocations++;
                ClassReflectFunctionInvoker<Derived, T>::call(*This(), obj);
                numInvocations--;
                if(numInvocations == 0 && externalInvocation)
                {
                    this->This()->onInvocationEnd(obj);
                }
            }

            // Calls the reflect function of the given object.
            template<typename T>
            void invokeReflectOnObject(const char* name, T& obj, bool externalInvocation = true)
            {
                // externalInvocation: Did an external function apply this reflector an object,
                // or was invokeReflectOnObject called by ourselves internally, e.g., by the ClassInvoker?
                if(numInvocations == 0 && externalInvocation)
                {
                    this->This()->onInvocationStart(obj);
                }
                numInvocations++;
                //ReflectorInvoker<Derived, T>::call(*This(), obj);

                typedef typename std::remove_const<T>::type NonConstType; 
                NonConstType& non_const_member = *const_cast<NonConstType*>(&obj);
                this->member(name, non_const_member, "");
                numInvocations--;
                if(numInvocations == 0 && externalInvocation)
                {
                    this->This()->onInvocationEnd(obj);
                }
            
            }

            template<typename T>
            void onInvocationStart(T& obj)
            {
                // Default implementation
            }

            template<typename T>
            void onInvocationEnd(T& obj)
            {
                // Default implementation
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
            void member(const char* property, T& member, const char* comment, const T& defaultValue)
            {
                this->currentDefaultValue = const_cast<T*>(&defaultValue);
                ReflectorType<T>::call(property, *this->This(), member);
            }

            template<typename T>
            void member(const char* property, Getter<T> getter, Setter<T> setter)
            {
                VariableWithGetterSetter<T> var(property, getter, setter);

                // This will invoke the reflector invoker for classes, which therefore will call the reflect function of VariableWithGetterSetter,
                // which forwards the reflector to the underlying type T. See how ClassInvoker treats VariableWithGetterSetter for more details.
                ReflectorType<VariableWithGetterSetter<T>>::call(property, *this->This(), var);
            }

            template<typename T>
            void member(const char* property, Getter<T> getter, Setter<T> setter, const T& defaultValue)
            {
                VariableWithGetterSetter<T> var(property, getter, setter, &defaultValue);
                this->currentDefaultValue = const_cast<T*>(&defaultValue);

                // This will invoke the reflector invoker for classes, which therefore will call the reflect function of VariableWithGetterSetter,
                // which forwards the reflector to the underlying type T. See how ClassInvoker treats VariableWithGetterSetter for more details.
                ReflectorType<VariableWithGetterSetter<T>>::call(property, *this->This(), var);
            }

            // Determines the type of the object (can be anything, primitive or class),
            // and decies which function to call in the reflector (e.g. callInt, callFloat, ...)
            template<typename T>
            void callAppropriateFunctionBasedOnType(const char* name, T& obj)
            {
                ReflectorType<T>::call(name, *this->This(), obj);
            }

            Derived* This()
            {
                return static_cast<Derived*>(this);
            }

    };  
}

