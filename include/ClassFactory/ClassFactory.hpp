#pragma once


#include "ClassFactoryBase.hpp"
#include "ClassFactoryTyped.hpp"

#include "Exception/Exception.hpp"
#include "Utilities/Singleton.hpp"

#include "TypeChecking/TypeCheckingFunctions.hpp"

#include <map>
#include <string>

namespace portaible
{
	namespace ClassFactory
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
						PORTAIBLE_THROW(portaible::Exception, "Error, class \"" << className << "\" was registered to ClassFactory more than once");
					}

					classFactories.insert(std::make_pair(className, static_cast<ClassFactoryBase*>(new ClassFactoryTyped<T>)));
				

					// The rttiString is not necessarily the same as class name (however, can be, depending on the compiler).
					std::string rttiString = getDataTypeRTTIString<T>();
					auto it2 = rttiToRegisteredClassNameMap.find(rttiString);


					if (it2 != rttiToRegisteredClassNameMap.end())
					{
						PORTAIBLE_THROW(portaible::Exception, "Error, class \"" << className << "\" was registered to the ClassFactory for the first time, however it's RTTI was already memorized for another type."
						"This should never happen and is either a serious programming mistake OR some compiler weirdness, which leads to mapping two different data types to the RTTI string.");
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
					
					T* obj = static_cast<T*>(getFactoryForClassByName(className)->getInstanceUntyped());
					return obj;
				}
		};

		template<typename T>
		class RegisterHelper
		{

			public:
				RegisterHelper(std::string name) 
				{
					ClassFactory::getInstance()->registerFactory<T>(name);
				}

	
		};

	}
}


#define DECLARE_CLASS_FACTORY(className) \
	private:\
	static volatile portaible::ClassFactory::RegisterHelper<className> classFactoryRegistrar;\
	static const std::string __CLASS_NAME__;\
	public:\
	const virtual std::string& getClassName() const {return className::__CLASS_NAME__;}\
	static std::string staticGetGlassName() {return className::__CLASS_NAME__;}

#define REGISTER_TO_CLASS_FACTORY(className) \
	volatile portaible::ClassFactory::RegisterHelper<className> className::classFactoryRegistrar (#className);\
	const std::string className::__CLASS_NAME__ = #className;
