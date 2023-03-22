#pragma once

#include <functional>

namespace claid
{
    

    template<class T>
    class Setter
    {
      
        private:
            std::function<void (const T&)> setterFunction;

        public:

            Setter(std::function<void (const T&)> setterFunction) : setterFunction(setterFunction)
            {

            }

            Setter(std::function<void (T)> setterFunction) : setterFunction(setterFunction)
            {

            }

            template<typename Class, typename... Params>
            Setter(void (Class::*setter)(Params..., const T&), Class* obj, Params... params) 
            {
                this->setterFunction = std::bind(setter, *obj, params..., std::placeholders::_1);
            }

    

            Setter& operator=(const T& value)
            {
                setterFunction(value);
                return *this;
            }

            Setter operator()(const T& value)
            {
                setterFunction(value);
                return *this;
            }
   
   
    };

    
    
}