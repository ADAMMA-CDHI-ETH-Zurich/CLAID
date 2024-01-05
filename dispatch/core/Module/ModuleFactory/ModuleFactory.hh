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


#include "ModuleFactoryBase.hh"
#include "ModuleFactoryTyped.hh"

#include "dispatch/core/Utilities/Singleton.hh"

#include <map>
#include <string>
#include <vector>

namespace claid
{
	class ModuleFactory : public Singleton<ModuleFactory>
	{
		private:
			std::map<std::string, ModuleFactoryBase*> moduleFactories;

		public:
			template<typename T>
			void registerFactory(const std::string& className)
			{
				static_assert(std::is_base_of<claid::Module, T>::value, "Tried to register a class as Module (see above), which did not inherit from Module."); 
				ModuleFactoryBase* factory = static_cast<ModuleFactoryBase*>(new ModuleFactoryTyped<T>);
				registerFactory(className, factory);
			}

			void registerFactory(const std::string& className, ModuleFactoryBase* moduleFactory)
			{
				auto it = moduleFactories.find(className);

				if (it != moduleFactories.end())
				{
					// Not an error. This might happen when importing shared libraries that also were build with CLAID (e.g., importing PyCLAID from a PythonModule).
					return;
					//CLAID_THROW(claid::Exception, "Error, class \"" << className << "\" was registered to ModuleFactory more than once");
				}
				moduleFactories.insert(std::make_pair(className, moduleFactory));
			}

			bool isModuleClassRegistered(const std::string& className)
			{
				auto it = moduleFactories.find(className);

				return it != moduleFactories.end();
			}

			ModuleFactoryBase* getFactoryForModule(const std::string& className)
			{
				if (!isModuleClassRegistered(className))
				{
					return nullptr;
				}

				auto it = moduleFactories.find(className);
				return it->second;
			}

			template<typename T>
			T* getNewInstanceAndCast(const std::string& className)
			{
				if (!isModuleClassRegistered(className))
				{
					return nullptr;
				}
				
				T* obj = dynamic_cast<T*>(getFactoryForModule(className)->getInstanceUntyped());
				return obj;
			}

			Module* getInstanceUntyped(const std::string& className)
			{
				if (!isModuleClassRegistered(className))
				{
					return nullptr;
				}

				auto it = moduleFactories.find(className);
				return it->second->getInstanceUntyped();
			}

			std::set<std::string> getRegisteredModuleClasses()
			{
				std::set<std::string> output;

				for(auto it : this->moduleFactories)
				{
					output.insert(it.first);
				}

				return output;
			}

			bool getExpectedPropertiesOfModule(const std::string& moduleName, std::map<std::string, std::string>& expectedProperties)
			{
				auto it = moduleFactories.find(moduleName);

				if (it == moduleFactories.end())
				{
					return false;
				}

				return it->second->getExpectedPropertiesOfModule(expectedProperties);
			}
	};

	template<typename T>
	class ModuleFactoryRegistrar
	{
		public:
			ModuleFactoryRegistrar(std::string name) 
			{
				std::string moduleClassWithoutNamespace = getModuleNameFromClassName(name);
				ModuleFactory::getInstance()->registerFactory<T>(moduleClassWithoutNamespace);
			}	

			static std::string getModuleNameFromClassName(std::string className)
			{
				// If className is something like namespace::Module, remove namespace:: and return Module.
				// Otherwise, if className is Module (e.g. MicrophoneDataModule), just return Module.
				size_t index = className.find_last_of("::");
				if(index == std::string::npos)
				{
					return className;
				}

				// Return everything after the last occurence of ::
				return className.substr(index + 1, className.size());
			}
	};

}


// #define DECLARE_MODULE_FACTORY(className) 
// 	static volatile claid::ModuleFactory::RegisterHelper<className> moduleFactoryRegistrar;
// 	static const std::string __MODULE_NAME__;
// 	const std::string getModuleName() {return className::__MODULE_NAME__;}

// Uses anonymous namespace to solve multiple definitions issue; c.f. https://artificial-mind.net/blog/2020/10/17/static-registration-macro (great article)
#define REGISTER_MODULE_FACTORY(className) \
    namespace \
    { \
        static volatile claid::ModuleFactoryRegistrar<className> moduleFactoryRegistrar (#className); \
    }

#define REGISTER_MODULE_FACTORY_CUSTOM_NAME(moduleName, className) \
    namespace \
    { \
        static volatile claid::ModuleFactoryRegistrar<className> moduleFactoryRegistrar##moduleName (#moduleName); \
    }

