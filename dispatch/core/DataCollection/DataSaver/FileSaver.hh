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
#include "dispatch/core/CLAID.hh"
#include "dispatch/core/Utilities/Path.hh"

#include <fstream>

namespace claid
{
    class DataSaverModule;

    class FileSaver
    {
        private:
            std::shared_ptr<AbstractSerializer> serializer;
            std::string what;
            std::string storagePath;
            std::string fileNameFormat;
            bool excludeHeader;
            
            std::string tmpStoragePath;
            std::string currentFilePath;

            std::ofstream currentFile;

            DataSaverModule* parentModule = nullptr;
            Path currentPath;
            Channel<Untyped> dataChannel;

        public:

            // template<typename Reflector>
            // void reflect(Reflector& reflector)
            // {
            // reflector.member("serializer", serializer, "");

            // }
   

            std::string getCurrentFilePath();

            FileSaver();
            FileSaver(DataSaverModule* parentModule);

            void initialize(const std::map<std::string, std::string>& properties, DataSaverModule* parentModule);
            void onData(ChannelData<Untyped> data);
            void storeData(ChannelData<Untyped>& d);
            void storeDataHeader(const Path& path);
    
            void getCurrentPathRelativeToStorageFolder(Path& path, const Time timestamp);
            void createDirectoriesRecursively(const std::string& path);
            void beginNewFile(const Path& path);
            void openFile(const std::string& path);
            

            void moveTemporaryFilesToStorageDestination();

            void createStorageFolder(const Path& currentSavePath);
            void createTmpFolderIfRequired(const Path& currentSavePath);


    };
}