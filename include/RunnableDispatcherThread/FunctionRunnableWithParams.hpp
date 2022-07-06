#pragma once

#include "FunctionRunnableBase.hpp"
#include <functional>

namespace portaible
{
    template<typename Return, typename... Ts>
    class FunctionRunnableWithParams : public FunctionRunnableBase
    {
        private:
            std::function<Return (Ts...)> function;
            std::tuple<Ts...> stack;

            void run()
            {
                std::apply(this->function, this->stack);
            }

        public:
            FunctionRunnableWithParams(std::function<Return (Ts...)> function) : function(function)
            {

            }

            void setParams(Ts... params)
            {
                this->stack = {params...};
            }

        
    };
}