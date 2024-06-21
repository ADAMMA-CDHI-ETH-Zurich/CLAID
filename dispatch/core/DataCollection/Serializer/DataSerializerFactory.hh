/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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
				Logger::logInfo("Num serializers: %d\n", this->dataTypeToSerializerMap.size());
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

