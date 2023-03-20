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

            typedef T GetterValueType;

            Getter(std::function<T()> getterFunction) : getterFunction(getterFunction)
            {

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