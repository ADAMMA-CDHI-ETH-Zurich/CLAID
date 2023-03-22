#pragma once
#include "Getter.hpp"
#include "Setter.hpp"

namespace claid
{
    

    template<typename T>
    class VariableWithGetterSetter
    {

        private:
            Getter<T> getter;
            Setter<T> setter;
            std::string variableName;

     

        public:

            typedef typename Getter<T>::GetterValueType GetterValueType;
            typedef T ValueType;
           

            VariableWithGetterSetter(std::string variableName, Getter<T> getter, Setter<T> setter) : variableName(variableName), getter(getter), setter(setter)
            {
                
            }

            VariableWithGetterSetter& operator=(const T& other)
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

            template<typename Reflector>
            void reflect(Reflector& r)
            {
                splitReflectInType(r, *this);
            }

            template<typename Reflector>
            void reflectRead(Reflector& r)
            {
                T tmp;
                r.member(this->variableName.c_str(), tmp, "");
                setter(tmp);
            }

            template<typename Reflector>
            void reflectWrite(Reflector& r)
            {
                T tmp = getter;
                r.member(this->variableName.c_str(), tmp, "");
            }

          
    };

  

}