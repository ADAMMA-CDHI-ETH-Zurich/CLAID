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

            template<typename Class>
            Setter(void (Class::*setter)(const T&), Class& obj) 
            {
                this->setterFunction = std::bind(setter, obj);
            }

            template<typename Class>
            Setter(void (Class::*setter)(T), Class& obj) 
            {
                this->setterFunction = std::bind(setter, obj);
            }

            Setter& operator=(const T& value)
            {
                setterFunction(value);
                return *this;
            }
   
   
    };

    
    
}