/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions
* 
* Authors: Patrick Langer
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