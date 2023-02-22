#pragma once


#include "ModuleFactoryBase.hpp"
#include "ModuleFactoryTyped.hpp"

#include "Exception/Exception.hpp"
#include "Utilities/Singleton.hpp"

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

			bool isFactoryRegisteredForModule(const std::string& moduleName)
			{
				auto it = moduleFactories.find(moduleName);

				return it != moduleFactories.end();
			}

			ModuleFactoryBase* getFactoryForModuleByName(const std::string& moduleName)
			{
				if (!isFactoryRegisteredForModule(moduleName))
				{
					return nullptr;
				}

				auto it = moduleFactories.find(moduleName);
				return it->second;
			}

			template<typename T>
			T* getNewInstanceAndCast(const std::string& moduleName)
			{
				if (!isFactoryRegisteredForModule(moduleName))
				{
					return nullptr;
				}
				
				T* obj = dynamic_cast<T*>(getFactoryForModuleByName(moduleName)->getInstanceUntyped());
				return obj;
			}

			std::vector<std::string> getRegisteredModuleNames()
			{
				std::vector<std::string> output;

				for(auto it : this->moduleFactories)
				{
					output.push_back(it.first);
				}

				return output;
			}
	};

	template<typename T>
	class ModuleFactoryRegistrar
	{
		public:
			ModuleFactoryRegistrar(std::string name) 
			{
				ModuleFactory::getInstance()->registerFactory<T>(name);
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


// #define DECLARE_MODULE_FACTORY(className) \
// 	static volatile claid::ModuleFactory::RegisterHelper<className> moduleFactoryRegistrar;\
// 	static const std::string __MODULE_NAME__;\
// 	const std::string getModuleName() {return className::__MODULE_NAME__;}

// Uses anonymous namespace to solve multiple definitions issue; c.f. https://artificial-mind.net/blog/2020/10/17/static-registration-macro (great article)
#define REGISTER_MODULE_FACTORY(className)\
	namespace\
	{\
		static volatile claid::ModuleFactoryRegistrar<className> moduleFactoryRegistrar (#className);\
	}\
