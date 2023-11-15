/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions
* 
* Authors: Patrick Langer
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

            FunctionRunnableWithParams(std::function<Return (Ts...)> function) : function(function)
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