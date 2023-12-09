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

#pragma once

#include "dispatch/core/DataCollection/Serializer/DataSerializer.hh"
#include "dispatch/core/Utilities/Path.hh"
#include "dispatch/core/Module/ChannelData.hh"

#include <fstream>

namespace claid
{
    class DataSaverModule;

    class FileSaver
    {
        private:
            std::shared_ptr<DataSerializer> serializer;
            std::string what;
            std::string storagePath;
            std::string fileNameFormat;
            std::string fileType;

            bool excludeHeader;
            
            std::string tmpStoragePath;
            std::string currentFilePath;


            Path currentPath;

            std::string getCurrentFilePath();
            void getCurrentPathRelativeToStorageFolder(Path& path, const Time timestamp);
            absl::Status createDirectoriesRecursively(const std::string& path);
            absl::Status beginNewFile(const Path& path);
            

            void moveTemporaryFilesToStorageDestination();

            absl::Status createStorageFolder(const Path& currentSavePath);
            absl::Status createTmpFolderIfRequired(const Path& currentSavePath);

            bool initialized = false;

            bool hasReceivedData = false;

        public:

            // template<typename Reflector>
            // void reflect(Reflector& reflector)
            // {
            // reflector.member("serializer", serializer, "");

            // }
   

            

            FileSaver();

            absl::Status initialize(const std::string& what, const std::string& storagePath, const std::string& fileNameFomat, const std::string& fileType);
            absl::Status onNewData(ChannelData<google::protobuf::Message>& data);
            absl::Status onNewData(std::shared_ptr<const google::protobuf::Message> data, const Time& timestamp);

            absl::Status endFileSaving();

            // void storeDataHeader(const Path& path);
                
            

    };
}