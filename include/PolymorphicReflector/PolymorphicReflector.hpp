#pragma once

#include "Utilities/Singleton.hpp"
#include "WrappedReflectorTyped.hpp"

#include <map>
#include <string>

namespace claid
{
	namespace PolymorphicReflector
	{
		template<typename Reflector> 
		class PolymorphicReflector : public Singleton<PolymorphicReflector<Reflector>>
		{
			private:

				//std::map<std::string /* Class name */, GenericReflector* /* Available Reflectors for class */> reflectors;
				std::map<std::string, WrappedReflectorBase<Reflector>*> reflectors;
			public:

				template<typename ObjectType>
				void registerReflector(std::string className)
				{
					auto it = reflectors.find(className);

					if (it != reflectors.end())
					{
						return;
						//CLAID_THROW(claid::Exception, "Error, PolymorhpReflector for class \"" << className << "\" was registered multiple times.");
					}

					WrappedReflectorTyped<ObjectType, Reflector>* reflector = new WrappedReflectorTyped<ObjectType, Reflector>;
					reflectors.insert(std::make_pair(className, static_cast<WrappedReflectorBase<Reflector>*>(reflector)));
				}

				// template<typename Class, typename Reflector>
				// void getReflectorForClass()
				// {

				// }

				bool getReflector(const std::string& className, WrappedReflectorBase<Reflector>*& reflector)
				{
					auto it = this->reflectors.find(className);

					if (it == this->reflectors.end())
					{
						return false;
					}

					reflector = it->second;
					return true;
				}
		};

		template<typename ObjectType, typename Reflector>
		class RegisterHelper
		{
		public:
			RegisterHelper(std::string name)
			{
				PolymorphicReflector<Reflector>::getInstance()->template registerReflector<ObjectType>(name);
			}
		};
	}
	
}


// #define DECLARE_POLYMORPHIC_REFLECTOR(className, Reflector, ReflectorName) \
//      static claid::PolymorphicReflector::RegisterHelper<className, Reflector> polymorphicReflectorRegistrar##ReflectorName;


#define REGISTER_POLYMORPHIC_REFLECTOR(className, Reflector, ReflectorName)\
	namespace\
	{\
		static claid::PolymorphicReflector::RegisterHelper<className, Reflector> polymorphicReflectorRegistrar##ReflectorName (#className);\
	}