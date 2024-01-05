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

#include "dispatch/core/Module/ModuleFactory/ModuleFactoryBase.hh"



namespace claid
{


	template <typename T>
	struct has_define_expected_properties_function {
		template <typename U, typename = decltype(&U::defineExpectedProperties)>
		static std::true_type test(int);

		template <typename>
		static std::false_type test(...);

		static constexpr bool value = decltype(test<T>(0))::value;
	};


	template<typename T, class Enable = void>
	struct __DefineExpectedPropertiesFunctionInvoker 
	{
		static bool call(std::map<std::string, std::string>& properties) 
		{
			return false;
		}

	}; 

	template<class T>
	struct __DefineExpectedPropertiesFunctionInvoker<T, typename std::enable_if<has_define_expected_properties_function<T>::value>::type> 
	{
		static bool call(std::map<std::string, std::string>& properties) 
		{
			T::defineExpectedProperties(properties);
			return true;
		}
	};    


	template<typename T>
	class ModuleFactoryTyped : public ModuleFactoryBase
	{
		public:
			Module* getInstanceUntyped() override final
			{
				return static_cast<Module*>(new T);
			}

			T* getInstance()
			{
				return new T;
			}

			bool getExpectedPropertiesOfModule(std::map<std::string, std::string>& expectedProperties) override final
			{
				expectedProperties.clear();
				return __DefineExpectedPropertiesFunctionInvoker<T>::call(expectedProperties);
			}

	};
}
