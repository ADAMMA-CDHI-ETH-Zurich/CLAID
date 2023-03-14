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


    template<typename T>
    class ReflectedVariable
    {

        private:
            ReflectedVariableGetter<T> getter;
            ReflectedVariableSetter<T> setter;

        public:

            typedef typename ReflectedVariableGetter<T>::ValueType GetterValueType;

            ReflectedVariable(T& variable)
            {
                this->getter = ReflectedVariableGetter<T>(variable);
                this->setter = ReflectedVariableSetter<T>(variable);

            }

            ReflectedVariable(ReflectedVariableGetter<T> getter, ReflectedVariableSetter<T> setter) : getter(getter), setter(setter)
            {
                
            }

            ReflectedVariable<T>& operator=(const T& other)
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

}