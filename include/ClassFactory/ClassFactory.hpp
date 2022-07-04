#pragma once


#include "ClassFactoryBase.hpp"
#include "ClassFactoryTyped.hpp"

#include "Exception/Exception.hpp"
#include "Utilities/Singleton.hpp"

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
				}

				bool isFactoryRegisteredForClass(const std::string& className)
				{
					auto it = classFactories.find(className);

					return it != classFactories.end();
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
	static volatile portaible::ClassFactory::RegisterHelper<className> classFactoryRegistrar;\
	static const std::string __CLASS_NAME__;\
	const std::string& getClassName() {return className::__CLASS_NAME__;}

#define REGISTER_TO_CLASS_FACTORY(className) \
	volatile portaible::ClassFactory::RegisterHelper<className> className::classFactoryRegistrar (#className);\
	const std::string className::__CLASS_NAME__ = #className;
