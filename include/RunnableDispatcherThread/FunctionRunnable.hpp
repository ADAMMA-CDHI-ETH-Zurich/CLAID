#pragma once

#include "FunctionRunnableBase.hpp"
#include <functional>

namespace portaible
{
    template<typename Return>
    class FunctionRunnable : public FunctionRunnableBase
    {
        private:
            std::function<Return ()> function;
            void run()
            {
                this->function();
            }

        public:
            FunctionRunnable(std::function<Return ()> function) : function(function)
            {

            }

        
    };
}