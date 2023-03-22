#pragma once

#include "Utilities/Singleton.hpp"

#include <map>
#include <string>
#include "TypedReflector.hpp"
namespace claid
{
	class ReflectionManager : public Singleton<ReflectionManager>
	{
		private:

			std::map<std::string /* Class name */, std::map<std::string, UntypedReflector*> /* Available Reflectors for class */> reflectors;
		public:

			template<typename Class, typename Reflector>
			void registerReflectorForClass()
			{
				TypedReflector<Class, Reflector>* reflector = new TypedReflector<Class, Reflector>();

				std::string className = TypeChecking::getCompilerSpecificCompileTypeNameOfClass<Class>();
				std::string reflectorName = Reflector::getReflectorName();

				UntypedReflector* untypedReflector = static_cast<UntypedReflector*>(reflector);
				std::cout << "Registering class " << className << "\n";
				reflectors[className].insert(std::make_pair(reflectorName, untypedReflector));
			}

			// template<typename Class, typename Reflector>
			// void getReflectorForClass()
			// {

			// }

			template<typename Class, typename Reflector>
			bool getReflectorForClass(UntypedReflector*& reflector)
			{
				std::string className = TypeChecking::getCompilerSpecificCompileTypeNameOfClass<Class>();
				std::string reflectorName = Reflector::getReflectorName();

				return getReflectorForClass(className, reflectorName, reflector);
			}

			bool getReflectorForClass(const std::string& className, const std::string& reflectorName, UntypedReflector*& reflector)
			{
				std::cout << "Class name " << className << "\n";
				std::cout << "ReflectorName " << reflectorName << "\n";

				auto it = this->reflectors.find(className);

				if (it == this->reflectors.end())
				{
					return false;
				}
		
				
				auto it2 = it->second.find(reflectorName);
				
				if(it2 == it->second.end())
				{
					return false;
				}

				reflector = it2->second;

				return true;
			}
	};

	template<typename ObjectType, typename Reflector>
	class ReflectionManagerRegistrar
	{
	public:
		ReflectionManagerRegistrar()
		{
			std::cout << "Registrar helper\n";
			ReflectionManager::getInstance()->template registerReflectorForClass<ObjectType, Reflector>();
		}
	};
}


// #define DECLARE_POLYMORPHIC_REFLECTOR(className, Reflector, ReflectorName) \
//      static claid::PolymorphicReflector::RegisterHelper<className, Reflector> polymorphicReflectorRegistrar##ReflectorName;


#define REGISTER_REFLECTOR_FOR_CLASS(Class, Reflector, ReflectorName)\
	namespace\
	{\
		static volatile claid::ReflectionManagerRegistrar<Class, Reflector> reflectorRegistrar##ReflectorName;\
	}