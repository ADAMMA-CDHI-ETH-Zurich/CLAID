

#pragma once

#include "FunctionRunnableBase.hh"
#include "dispatch/core/Utilities/VariadicTemplateHelpers.hh"

namespace claid
{
    

   

    template<typename Return, typename... Ts>
    class FunctionRunnableWithParams : public FunctionRunnableBase
    {
        private:
            

            std::function<Return (Ts...)> function;
            std::tuple<Ts...> stack;

   


            void run()
            {
                VariadicTemplateHelpers::applyTupleToFunction(stack, function);
            }

        public:
            FunctionRunnableWithParams()
            {
                
            }

            FunctionRunnableWithParams(std::function<Return (Ts...)> function) : function(function), stack()
            {
            }

            template<class Class, class... Args>
            void bind(void (Class::*p)(Args...), Class* obj)
            {
                this->function = VariadicTemplateHelpers::bind_with_variadic_placeholders<Class, Args...>(p, obj);
            }

            void setParams(Ts... params)
            {
                this->stack = {params...};
            }
            
            template<class Class, class... Args>
            void bindWithParams(void (Class::*p)(Args...), Class* obj, Args... args)
            {
                this->bind(p, obj);
                this->setParams(args...);
            }

        
    };
}