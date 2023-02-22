#pragma once


#include "ClassFactoryBase.hpp"
#include "ClassFactoryTyped.hpp"

#include "Exception/Exception.hpp"
#include "Utilities/Singleton.hpp"

#include "TypeChecking/TypeCheckingFunctions.hpp"

#include <map>
#include <string>

namespace claid
{
	class ClassFactory : public Singleton<ClassFactory>
	{
		private:
			std::map<std::string, ClassFactoryBase*> classFactories;

			// RTTI map to registered class name.
			// Needed to verify whether a derived type was registered
			// to use serialization (i.e. register to ClassFactory and PolymorphicReflector).
			std::map<std::string, std::string> rttiToRegisteredClassNameMap;

		public:
			template<typename T>
			void registerFactory(const std::string& className)
			{
				auto it = classFactories.find(className);

				if (it != classFactories.end())
				{
					// Not an error. This might happen when importing shared libraries that also were build with CLAID (e.g., importing PyCLAID from a PythonModule).
					return;
					//CLAID_THROW(claid::Exception, "Error, class \"" << className << "\" was registered to ClassFactory more than once");
				}

				classFactories.insert(std::make_pair(className, static_cast<ClassFactoryBase*>(new ClassFactoryTyped<T>)));
			

				// The rttiString is not necessarily the same as class name (however, can be, depending on the compiler).
				std::string rttiString = TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>();
				auto it2 = rttiToRegisteredClassNameMap.find(rttiString);


				if (it2 != rttiToRegisteredClassNameMap.end())
				{
					CLAID_THROW(claid::Exception, "Error, class with className \"" << className << "\" was registered to the ClassFactory for the first time and it's compiler specific RTTI name is \"" << rttiString << "\".\n" <<
					"But this compiler specific RTTI name was already memorized when registering another class, \"" << it->second << "\"." 
					"This should never happen and is either a serious programming mistake OR some compiler weirdness, which leads to mapping two different data types to the same RTTI string.");
				}
				rttiToRegisteredClassNameMap.insert(std::make_pair(rttiString, className));
			}

			bool isFactoryRegisteredForClass(const std::string& className)
			{
				auto it = classFactories.find(className);

				return it != classFactories.end();
			}

			bool isFactoryRegisteredForRTTITypeName(const std::string& rttiTypename)
			{
				auto it = rttiToRegisteredClassNameMap.find(rttiTypename);

				return it != rttiToRegisteredClassNameMap.end();
			}

			ClassFactoryBase* getFactoryForClassByName(const std::string& className)
			{
				if (!isFactoryRegisteredForClass(className))
				{
					return nullptr;
				}

				auto it = classFactories.find(className);
				return it->second;
			}

			template<typename T>
			T* getNewInstanceAndCast(const std::string& className)
			{
				if (!isFactoryRegisteredForClass(className))
				{
					return nullptr;
				}
				
				// ClassFactoryTyped<T>* factory = dynamic_cast<ClassFactoryTyped<T>*>(getFactoryForClassByName(className));

				// if(factory == nullptr)
				// {
				// 	CLAID_THROW(Exception, "Error in ClassFactory. Tried to create a new object of class \"" << TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>() << "\" from "
				// 	<< "factory associated with type \"" << className << "\". The types are incompatible, the latter cannot be cast to the former.");
				// }

				T* obj = static_cast<T*>(getFactoryForClassByName(className)->getInstanceUntyped());
				return obj;
			}

			void getRegisteredClasses(std::vector<std::string>& registeredClasses)
			{
				for(auto it : classFactories)
				{
					registeredClasses.push_back(it.first);
				}
			}

			template<typename T>
			std::string getClassNameOfObject(const T& instance)
			{
				std::string rttiTypename = TypeChecking::getCompilerSpecificRunTimeNameOfObject<T>(instance);
				auto it = rttiToRegisteredClassNameMap.find(rttiTypename);

				if(it == rttiToRegisteredClassNameMap.end())
				{
					CLAID_THROW(Exception, "Cannot get registered class name for type \"" << rttiTypename << "\". This type was not registered to the class factory using REGISTER_CLASS");
				}
				else
				{
					return it->second;
				}
			}

			template<typename T>
			std::string getClassName()
			{
				std::string rttiTypename = TypeChecking::getCompilerSpecificCompileTypeNameOfClass<T>();
				auto it = rttiToRegisteredClassNameMap.find(rttiTypename);

				if(it == rttiToRegisteredClassNameMap.end())
				{
					CLAID_THROW(Exception, "Cannot get registered class name for type \"" << rttiTypename << "\". This type was not registered to the class factory using REGISTER_CLASS");
				}
				else
				{
					return it->second;
				}
			}
	};

	template<typename T>
	class ClassFactoryRegistrar
	{

		public:
			ClassFactoryRegistrar(std::string name) 
			{
				ClassFactory::getInstance()->registerFactory<T>(name);
			}


	};

}




// Uses anonymous namespace to solve multiple definitions issue; c.f. https://artificial-mind.net/blog/2020/10/17/static-registration-macro (great article)
#define REGISTER_TO_CLASS_FACTORY(className) \
	namespace\
	{\
		volatile claid::ClassFactoryRegistrar<className> classFactoryRegistrar (#className);\
	}


