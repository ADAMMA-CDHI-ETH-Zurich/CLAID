
#pragma once

#include "dispatch/core/DataCollection/Serializer/DataSerializerFactory.hh"

namespace claid
{
	template<typename T>
	class DataSerializerFactoryTyped : public DataSerializerFactoryBase
	{

		public:
			DataSerializerFactoryTyped(const std::vector<std::string> dataFormatNames) : DataSerializerFactoryBase(dataFormatNames)
			{

			}

			std::shared_ptr<DataSerializer> getInstanceUntyped()
			{
				return std::static_pointer_cast<DataSerializer>(std::make_shared<T>());
			}

			std::shared_ptr<T> getInstance()
			{
				return std::make_shared<T>();
			}
	};
}