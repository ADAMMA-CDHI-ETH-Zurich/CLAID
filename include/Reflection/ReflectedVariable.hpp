#pragma once
#include "Getter.hpp"
#include "Setter.hpp"

namespace claid
{
    

    template<typename T>
    class ReflectedVariableGetter
    {
        // Direct access
        private:
            const T& reference;

        public:
            typedef const T& ValueType;

            ReflectedVariableGetter(const T& reference) : reference(reference)
            {

            }

            ValueType get() const
            {
                return this->reference;
            }

            operator ValueType() const
            {
                return this->get();
            }
    };

    template<typename T>
    class ReflectedVariableGetter<Getter<T>>
    {
        // Access via separate getter
        private:
            Getter<T> getter;

        public:
            typedef T ValueType;

            ReflectedVariableGetter(Getter<T> getter) : getter(getter)
            {

            }

            operator ValueType() const
            {
                return getter();
            }
    };

    template<typename T>
    class ReflectedVariableSetter
    {
        // Direct access
        private:
            T& reference;

        public:

            ReflectedVariableSetter(T& reference) : reference(reference)
            {

            }

            ReflectedVariableSetter<T>& operator= (const T& other)
            {
                reference = other;
                return *this;
            }

            ReflectedVariableSetter<T> operator()(const T& other)
            {
                reference = other;
                return *this;
            }
    };

    template<typename T>
    class ReflectedVariableSetter<Setter<T>>
    {
        // Access via separate setter
        private:
            Setter<T> setter;

        public:
            typedef T ValueType;

            ReflectedVariableSetter(Setter<T> setter) : setter(setter)
            {

            }

            ReflectedVariableSetter<Setter<T>>& operator= (const T& other)
            {
                setter(other);
                return *this;
            }
    };


    template<typename T, typename GetterType = T, typename SetterType = T>
    class ReflectedVariable
    {

        private:
            ReflectedVariableGetter<GetterType> getter;
            ReflectedVariableSetter<SetterType> setter;
            std::string variableName;

     

        public:

            typedef typename ReflectedVariableGetter<GetterType>::ValueType GetterValueType;
            typedef T ValueType;
           

            ReflectedVariable(std::string variableName, ReflectedVariableGetter<GetterType> getter, ReflectedVariableSetter<SetterType> setter) : variableName(variableName), getter(getter), setter(setter)
            {
                
            }

            ReflectedVariable& operator=(const T& other)
            {
                setter(other);
                return *this;
            }

            GetterValueType get() const
            {
                return getter.get();
            }

            operator GetterValueType() const
            {
                return this->get();
            }

          
    };

    template<typename T>
    typename std::enable_if<!std::is_const<T>::value, ReflectedVariable<T>>::type 
    static inline make_reflected_variable(const char* name, T& member)
    {
        ReflectedVariableGetter<T> getter(member);
        ReflectedVariableSetter<T> setter(member);
        return ReflectedVariable<T>(name, getter, setter);
    }

    template<typename T>
    typename std::enable_if<std::is_const<T>::value, ReflectedVariable<typename std::remove_const<T>::type>>::type 
    static inline make_reflected_variable(const char* name, T& member)
    {
        typedef typename std::remove_const<T>::type NonConstType; 
        NonConstType& non_const_member = *const_cast<NonConstType*>(&member);


        return ReflectedVariable<NonConstType>(name, non_const_member);
    }

}