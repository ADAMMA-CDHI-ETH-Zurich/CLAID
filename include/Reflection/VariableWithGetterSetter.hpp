#pragma once
#include "Getter.hpp"
#include "Setter.hpp"
#include <string>
namespace claid
{
    

    template<typename T>
    class VariableWithGetterSetter
    {

        private:
            Getter<T> getter;
            Setter<T> setter;
            std::string variableName;
            const T* defaultValue;

     

        public:

            typedef typename Getter<T>::GetterValueType GetterValueType;
            typedef T ValueType;
           

            VariableWithGetterSetter(std::string variableName, 
            Getter<T> getter, Setter<T> setter,
            const T* defaultValue = nullptr) : variableName(variableName), getter(getter), setter(setter), defaultValue(defaultValue)
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
                if(defaultValue != nullptr)
                {   
                    r.member(this->variableName.c_str(), tmp, "", *defaultValue);
                }
                else
                {
                    r.member(this->variableName.c_str(), tmp, "");
                }
                setter(tmp);
            }

            template<typename Reflector>
            void reflectWrite(Reflector& r)
            {
                T tmp = getter;
                if(defaultValue != nullptr)
                {   
                    r.member(this->variableName.c_str(), tmp, "", *defaultValue);
                }
                else
                {
                    r.member(this->variableName.c_str(), tmp, "");
                }            
            }

          
    };

  

}