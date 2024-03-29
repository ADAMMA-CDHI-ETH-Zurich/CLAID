
#pragma once

#include "dispatch/core/DataCollection/Serializer/DataSerializer.hh"

namespace claid
{
	class DataSerializerFactoryBase
	{
		private:
			// Name of data formats supported by the serializer, e.g.: ["JSON", "json"];
			std::vector<std::string> dataTypeNames;

		public:
			DataSerializerFactoryBase(const std::vector<std::string>& dataTypeNames) : dataTypeNames(dataTypeNames)
			{

			}

			virtual ~DataSerializerFactoryBase() {}
			virtual std::shared_ptr<DataSerializer> getInstanceUntyped() = 0;
	
			bool doesSupportDataType(const std::string dataType)
			{
				return std::find(dataTypeNames.begin(), dataTypeNames.end(), dataType) != dataTypeNames.end();
			}
	};
}