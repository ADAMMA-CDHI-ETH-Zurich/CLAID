
#pragma once


#include "DataSerializerFactoryBase.hh"
#include "DataSerializerFactoryTyped.hh"

#include "dispatch/core/Utilities/Singleton.hh"
#include "dispatch/core/Exception/Exception.hh"

#include <map>
#include <string>
#include <vector>

namespace claid
{
	class DataSerializerFactory : public Singleton<DataSerializerFactory>
	{
		private:
			std::map<std::string, DataSerializerFactoryBase*> dataSerializerFactories;
			std::map<std::string, std::string> dataTypeToSerializerMap;

		public:
			template<typename T>
			void registerFactory(const std::string& className, const std::vector<std::string>& dataTypeNames)
			{
				static_assert(std::is_base_of<claid::DataSerializer, T>::value, "Tried to register a class as DataSerializer (see above), which did not inherit from DataSerializer."); 
				DataSerializerFactoryBase* factory = static_cast<DataSerializerFactoryBase*>(new DataSerializerFactoryTyped<T>(dataTypeNames));
				registerFactory(className, factory, dataTypeNames);
			}

			void registerFactory(const std::string& className, DataSerializerFactoryBase* DataSerializerFactory, const std::vector<std::string>& dataTypeNames)
			{
				{
					auto it = dataSerializerFactories.find(className);

					if (it != dataSerializerFactories.end())
					{
						// Not an error. This might happen when importing shared libraries that also were build with CLAID (e.g., importing PyCLAID from a PythonModule).
						return;
						//CLAID_THROW(claid::Exception, "Error, class \"" << className << "\" was registered to DataSerializerFactory more than once");
					}
				}

				for(const std::string& dataType : dataTypeNames)
				{
					auto it = dataTypeToSerializerMap.find(dataType);
					if(it != dataTypeToSerializerMap.end())
					{
						CLAID_THROW(claid::Exception, "Error, cannot register serializer \"" << className << "\".\n"
									<< "The serializer supports the data type format \"" << dataType << "\"." );
					}
					dataTypeToSerializerMap.insert(std::make_pair(dataType, className));
				}

				dataSerializerFactories.insert(std::make_pair(className, DataSerializerFactory));
			}

			bool isFactoryClassRegistered(const std::string& className)
			{
				auto it = dataSerializerFactories.find(className);

				return it != dataSerializerFactories.end();
			}

			DataSerializerFactoryBase* getFactoryForDataSerializer(const std::string& className)
			{
				if (!isFactoryClassRegistered(className))
				{
					return nullptr;
				}

				auto it = dataSerializerFactories.find(className);
				return it->second;
			}

			template<typename T>
			std::shared_ptr<T> getNewInstanceAndCast(const std::string& className)
			{
				if (!isFactoryClassRegistered(className))
				{
					return nullptr;
				}
				
				std::shared_ptr<T> obj = std::static_pointer_cast<T>(getFactoryForDataSerializer(className)->getInstanceUntyped());
				return obj;
			}

			std::shared_ptr<DataSerializer> getInstanceUntyped(const std::string& className)
			{
				if (!isFactoryClassRegistered(className))
				{
					return nullptr;
				}

				auto it = dataSerializerFactories.find(className);
				return it->second->getInstanceUntyped();
			}

			std::shared_ptr<DataSerializer> getSerializerForDataType(const std::string& dataType)
			{
				auto it = this->dataTypeToSerializerMap.find(dataType);
				if(it == this->dataTypeToSerializerMap.end())
				{
					return nullptr;
				}

				return this->getInstanceUntyped(it->second);
			}

			std::set<std::string> getRegisteredDataSerializerClasses()
			{
				std::set<std::string> output;

				for(auto it : this->dataSerializerFactories)
				{
					output.insert(it.first);
				}

				return output;
			}
	};

	template<typename T>
	class DataSerializerFactoryRegistrar
	{
		public:
			DataSerializerFactoryRegistrar(std::string name, const std::vector<std::string>& dataTypeNames) 
			{
				DataSerializerFactory::getInstance()->registerFactory<T>(name, dataTypeNames);
			}	

			
	};

}



// Uses anonymous namespace to solve multiple definitions issue; c.f. https://artificial-mind.net/blog/2020/10/17/static-registration-macro (great article)

#define REGISTER_DATA_SERIALIZER(serializerName, className, dataTypes) \
    namespace \
    { \
        static volatile claid::DataSerializerFactoryRegistrar<className> DataSerializerFactoryRegistrar##serializerName (#serializerName, dataTypes); \
    }

