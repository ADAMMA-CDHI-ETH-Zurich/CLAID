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
                annotator.describeProperty("overrideExistingFiles", "If set to \"true\", an existing output file will be written whenever there is new data which would be stored in that file.", annotator.makeEnumProperty({"true", "false"}));
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