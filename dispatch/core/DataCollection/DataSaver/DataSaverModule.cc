/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller, Francesco Feher
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

#include "dispatch/core/DataCollection/DataSaver/DataSaverModule.hh"
namespace claid
{
    void DataSaverModule::initialize(const std::map<std::string, std::string>& propertiesMap)
    {
        PropertyHelper properties(propertiesMap);

        std::string what;
        std::string storagePath;
        std::string fileNameFormat;
        std::string fileType;


        properties.getProperty("what", what);
        properties.getProperty("storagePath", storagePath);
        properties.getProperty("fileNameFormat", fileNameFormat);
        properties.getProperty("fileType", fileType);

        if(properties.wasAnyPropertyUnknown())
        {
            std::string unknownProperties;
            properties.unknownPropertiesToString(unknownProperties);

            this->moduleError(absl::StrCat("Missing properties: [", unknownProperties, "]. Please sepcify the properties in the configuration file."));
        }

        absl::Status status = this->fileSaver.initialize(what, storagePath, fileNameFormat, fileType);
        if(!status.ok())
        {
            this->moduleError(status);
        }
    }
}

REGISTER_MODULE(DataSaverModule, claid::DataSaverModule)