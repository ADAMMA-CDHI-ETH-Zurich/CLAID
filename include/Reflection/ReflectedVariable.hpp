#pragma once
#include "Getter.hpp"
#include "Setter.hpp"

namespace claid
{
    template<typename T>
    class ReflectedVariable
    {

            
            ReflectedVariableGetter<T> getter;
            ReflectedVariableSetter<T> setter;

            ReflectedVariable(ReflectedVariableGetter<T> getter, ReflectedVariableSetter<T> setter) : getter(getter), setter(setter)
            {
                
            }

            operator=(const T& other)
            {
                setter(T);
            }

            getter::return_value_type get() const
            {
                return getter();
            }

            operator getter::return_value_type() const
            {
                return this->get();
            }
    };

    template<typename T>
    class ReflectedVariableGetter
    {
        // Direct access
        private:
            const T& reference;

        public:
            typedef const T& return_value_type;

            ReflectedVariableGetter(const T& reference) : reference(reference)
            {

            }

            operator return_value_type() const
            {
                return reference;
            }
    };

    template<typename T>
    class ReflectedVariableGetter<Getter<T>>
    {
        // Access via separate getter
        private:
            Getter<T> getter;

        public:
            typedef T return_value_type;

            ReflectedVariableGetter(Getter<T> getter) : getter(getter)
            {

            }

            operator return_value_type() const
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

            operator= (const T& other)
            {
                reference = other;
            }
    };

    template<typename T>
    class ReflectedVariableGetter<Getter<T>>
    {
        // Access via separate getter
        private:
            Setter<T> setter;

        public:
            typedef T return_value_type;

            ReflectedVariableGetter(Setter<T> setter) : setter(setter)
            {

            }

            operator= (const T& other)
            {
                setter(other);
            }
    };

}