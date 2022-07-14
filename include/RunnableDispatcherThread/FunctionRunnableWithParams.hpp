
#pragma once

#include "FunctionRunnableBase.hpp"
#include <functional>

// Needed for bind_with_variadic_arguments, see inside ChannelSynchronizer class
// See: https://stackoverflow.com/questions/21192659/variadic-templates-and-stdbind#21193316
template<int> // begin with 0 here!
struct placeholder_template
{
};

// Needs to be in namespace std!!
namespace std {

//////////////////////////////////////////////////////////////////////////////

template<int N>
struct is_placeholder<placeholder_template<N>>: integral_constant<int, N+1> // the 1 is important
{
};

//////////////////////////////////////////////////////////////////////////////

}
template<int ...>
struct sequence { };

template<int N, int ...S>
struct generateIntegerSequence : generateIntegerSequence<N-1, N-1, S...> { };

template<int ...S>
struct generateIntegerSequence<0, S...> {
typedef sequence<S...> type;
};

namespace portaible
{
    

   

    template<typename Return, typename... Ts>
    class FunctionRunnableWithParams : public FunctionRunnableBase
    {
        private:
            // bind_with_variadic_arguments (also please check at the beginning of this
            // file for definition of a specialized std is_placeholder type_trait).
            template<class Class, class... Args, int... Is>
            std::function<void (Args...)> bind_with_variadic_placeholders(void (Class::*p)(Args...), Class* obj, sequence<Is...>)
            {
                // std::function<void (Args...)>  y = std::bind(p, obj, boost::placeholders::_1, boost::placeholders::_2);
                std::function<void (Args...)> x = std::bind(p, obj, placeholder_template<Is>{}...);
                return x;
            }

            template<class Class, class... Args>
            std::function<void (Args...)> bind_with_variadic_placeholders(void (Class::*p)(Args...), Class* obj)
            {
                return bind_with_variadic_placeholders(p, obj, typename generateIntegerSequence<sizeof...(Args)>::type());
            }

            std::function<Return (Ts...)> function;
            std::tuple<Ts...> stack;

             template<int ...S>
            void applyTupleToFunction(sequence<S...>) 
            {
                function(std::get<S>(stack) ...);
            }

            void run()
            {
                applyTupleToFunction(typename generateIntegerSequence<sizeof...(Ts)>::type());
            }

        public:
            FunctionRunnableWithParams()
            {
                
            }

            FunctionRunnableWithParams(std::function<Return (Ts...)> function) : function(function)
            {
            }

            template<class Class, class... Args>
            void bind(void (Class::*p)(Args...), Class* obj)
            {
                this->function = bind_with_variadic_placeholders<Class, Args...>(p, obj);
            }

            void setParams(Ts... params)
            {
                this->stack = {params...};
            }

        
    };
}