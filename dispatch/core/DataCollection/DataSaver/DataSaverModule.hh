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
#include "dispatch/core/Module/Module.hh"
#include "dispatch/core/DataCollection/DataSaver/FileSaver.hh"
#include "dispatch/core/Module/TypeMapping/AnyProtoType.hh"
#include <fstream>

#include <google/protobuf/message.h>

namespace claid
{
    class DataSaverModule : public claid::Module
    {
        public:
            static void annotateModule(ModuleAnnotator& annotator)
            {
                annotator.setModuleCategory("DataCollection");
                annotator.setModuleDescription(absl::StrCat(
                    "The DataSaverModule allows to serialize data arriving on any channel to files on the local file system.\n",
                    "The Module can subscribe to ANY channel with any data type and automatically serialzes incoming data accordingly.\n",
                    "Different data formats (e.g., binary, json, ...) and file naming conventions (store data in one file per minute/hour/day etc.) are supported.\n"
                ));

                annotator.describeProperty("storagePath", "Path to a folder where to store files.", annotator.makePathProperty());
                annotator.describeProperty("fileNameFormat", "Naming convention for the files that data will be serialized to. You can use time format identifiers.\n"
                "For example, my_data_%H_%M.json would store data in one file per minute (e.g., my_data_12_30 for data recorded at 12:30).");
               
                annotator.describeProperty("fileType", "Data format for storing the data. Typically available are json, binary, batch_json and batch_binary.", annotator.makeEnumProperty({"json", "binary"}));
                annotator.describeProperty("overrideExistingFiles", "If set to \"true\", an existing output file will be written whenever there is new data which would be stored in that file.", annotator.makeEnumProperty({"false", "true"}));
                annotator.describeSubscribeChannel<int>("DataChannel", "Input channel for the data");
            }
        private:
            FileSaver fileSaver;
            Channel<AnyProtoType> dataChannel;

            void initialize(Properties properties);
            void terminate();

            void onData(ChannelData<AnyProtoType> data);

                

            
    };
}