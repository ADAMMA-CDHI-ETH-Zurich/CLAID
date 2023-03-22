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

            
            template<typename Class, typename... Params>
            Getter(T (Class::*getter)(Params...), Class* obj, Params... params) 
            {
                this->getterFunction = std::bind(getter, obj, params...);
            }

            T get() const
            {
                return getterFunction();
            }

            operator T() const
            {
                return this->get();
            }
   
   
    };

    
    
}