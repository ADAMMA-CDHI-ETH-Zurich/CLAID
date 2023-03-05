#pragma once

#include <functional>

namespace claid
{
    

    template<class T>
    class Getter
    {
      
        private:
            std::function<T ()> getterFunction;

        public:


            Getter(std::function<T()> getterFunction) : getterFunction(getterFunction)
            {

            }

            Getter(std::function<const T&()> getterFunction) : getterFunction(getterFunction)
            {

            }

            template<typename Class>
            Getter(const T& (Class::*getter)(), Class& obj) 
            {
                this->getterFunction = std::bind(getter, obj);
            }

            template<typename Class>
            Getter(T (Class::*getter)(), Class& obj) 
            {
                this->getterFunction = std::bind(getter, obj);
            }

            T get()
            {
                return getterFunction();
            }

            operator T() const
            {
                return this->get();
            }
   
   
    };

    
    
}