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