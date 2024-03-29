/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/


#pragma once

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


namespace claid
{
    template<int ...>
    struct sequence { };

    template<int N, int ...S>
    struct generateIntegerSequence : generateIntegerSequence<N-1, N-1, S...> { };

    template<int ...S>
    struct generateIntegerSequence<0, S...> {
    typedef sequence<S...> type;
    };

    struct VariadicTemplateHelpers
    {
        // bind_with_variadic_arguments (also please check at the beginning of this
        // file for definition of a specialized std is_placeholder type_trait).
        template<class Class, class... Args, int... Is>
        static std::function<void (Args...)> bind_with_variadic_placeholders(void (Class::*p)(Args...), Class* obj, sequence<Is...>)
        {
            // std::function<void (Args...)>  y = std::bind(p, obj, boost::placeholders::_1, boost::placeholders::_2);
            std::function<void (Args...)> x = std::bind(p, obj, placeholder_template<Is>{}...);
            return x;
        }

        template<class Class, class... Args>
        static std::function<void (Args...)> bind_with_variadic_placeholders(void (Class::*p)(Args...), Class* obj)
        {
            return bind_with_variadic_placeholders(p, obj, typename generateIntegerSequence<sizeof...(Args)>::type());
        }


        template<typename FunctionType, typename TupleType, int ...S>
        static void applyTupleToFunction(sequence<S...>, TupleType& tuple, FunctionType& function) 
        {
            function(std::get<S>(tuple) ...);
        }

        template<typename FunctionType, typename... Ts>
        static void applyTupleToFunction(std::tuple<Ts...>& tuple, FunctionType& function)
        {
            applyTupleToFunction(typename generateIntegerSequence<sizeof...(Ts)>::type(), tuple, function);
        }
    };
}