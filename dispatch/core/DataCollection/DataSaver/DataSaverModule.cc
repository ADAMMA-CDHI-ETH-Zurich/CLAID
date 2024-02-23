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
        bool overrideExistingFiles;


        properties.getProperty("what", what);
        properties.getProperty("storagePath", storagePath);
        properties.getProperty("fileNameFormat", fileNameFormat);
        properties.getProperty("fileType", fileType);
        properties.getOptionalProperty("overrideExistingFiles", overrideExistingFiles, false);

        Logger::logInfo("DataSaver override existing files: %d", overrideExistingFiles);

        if(properties.wasAnyPropertyUnknown())
        {
            std::string unknownProperties;
            properties.unknownPropertiesToString(unknownProperties);

            this->moduleFatal(absl::StrCat("Missing properties: [", unknownProperties, "]. Please sepcify the properties in the configuration file."));
            return;
        }


        absl::Status status = this->fileSaver.initialize(what, storagePath, fileNameFormat, fileType, overrideExistingFiles);
        if(!status.ok())
        {
            this->moduleFatal(status);
        }
        Logger::logInfo("DataChannel subscribe");
        this->dataChannel = this->subscribe(what, &DataSaverModule::onData, this);
    }

    void DataSaverModule::onData(ChannelData<AnyProtoType> data)
    {
        Logger::logInfo("DataSaverModule %s on data ", this->getId().c_str());
        const AnyProtoType& value = data.getData();
        std::shared_ptr<const google::protobuf::Message> msg = value.getMessage();

        absl::Status status = this->fileSaver.onNewData(msg, data.getTimestamp());
        if(!status.ok())
        {
            moduleError(status);
        }
    }

    void DataSaverModule::terminate()
    {
        absl::Status status = this->fileSaver.endFileSaving();
        if(!status.ok())
        {
            moduleError(status);
        }
    }

}

REGISTER_MODULE(DataSaverModule, claid::DataSaverModule)