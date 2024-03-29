
#pragma once

#include "FunctionRunnableBase.hh"
#include <functional>

namespace claid
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