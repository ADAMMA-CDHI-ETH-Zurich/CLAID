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
#include "dispatch/core/Utilities/FileUtils.hh"
#include "dispatch/core/Utilities/StringUtils.hh"
#include "dispatch/proto/sensor_data_types.grpc.pb.h"
#include "dispatch/proto/claidservice.grpc.pb.h"
// #ifdef __APPLE__
//     #include "TargetConditionals.h"
//     #if TARGET_OS_IPHONE
//     #include "CollectorAPI/iOSHelper/iOSApplicationPathHelper.hpp"
//     #endif
// #endif

#include "absl/strings/str_split.h"
#include "absl/status/status.h"

using claidservice::DataFile;

namespace claid
{
    // Syncs files contained in a certain directory with a DataReceiverModule.
    // This works in multiple steps:
    // 1st The DataSyncModule scans the specified directory and builds a list of available files.
    // 2nd This list is posted to a channel that the DataReceiverModule listens on.
    // 3rd The DataReceiverModule build a list of available files in the target directory aswell and compares
    // it with the list received from the DataSyncModule.
    // 4th The DataReceiverModule sends back a list of the files that are missing on it's side; this list is a subset
    // of the original list containing all files sent by the DataSyncModule.
    // 5th The DataSyncModule posts each file (in binary format) to the dataChannel, along with the target path.
    // 6th The DataReceiverModule saves each file.
    class DataSyncModule : public Module
    {
        public:
            static void annotateModule(ModuleAnnotator& annotator)
            {
                annotator.setModuleCategory("DataCollection");
                annotator.setModuleDescription(absl::StrCat(
                    "The DataSyncModule is the counterpart to the DataSyncModule. Together, the two Modules allow to synchronize files stored on the file system between different hosts.\n"
                    "Files can be synchronized based on a synchronization intervall (for example, every hour). Each file is tagged with the original host and user name, allowing you to\n"
                    "receive data from many different hosts and users and store it in separate folders."
                ));               
            }

        
        private:
            // On this channel, we post all the files available
            Channel<std::vector<std::string>> completeFileListChannel;

            // On this channel, the FileReceiver can request files that we will send.
            Channel<std::string> requestedFileChannel;

            // On this channel, we post the actual files.
            Channel<DataFile> dataFileChannel;

            // On this channel, the FileReceiver sends the name of the file that it received (acknowledgement).
            Channel<std::string> receivedFileAcknowledgementChannel;

            std::string filePath;

            Time lastMessageFromFileReceiver;
        
            std::string completeFileListChannelName;
            std::string requestedFileChannelName;
            std::string dataFileChannelName;
            std::string receivedFilesAcknowledgementChannelName;

            int syncingPeriodInMs;
            bool deleteFileAfterSync;

            void makePathRelative(std::string& path)
            {
                std::string basePath = this->filePath;
                if(this->filePath[this->filePath.size() - 1] != '/')
                {
                    basePath += '/';
                }

                if(StringUtils::startsWith(path, basePath))
                {
                    path = path.substr(basePath.size(), path.size());   
                }
            }

            void buildFileList(std::vector<std::string>& fileList)
            {
                fileList.clear();
                if(!FileUtils::getAllFilesInDirectoryRecursively(this->filePath, fileList))
                {
                    return;
                    //CLAID_THROW(Exception, "Error in DataSyncModule, cannot scan directory \"" << this->filePath << "\" "
                    //<< "for files. Directory either does not exist or we do not have reading permissions.");
                }

                for(std::string& path : fileList)
                {
                    makePathRelative(path);
                }

            }

            void sendFileList()
            {
                std::vector<std::string> fileList;
                this->buildFileList(fileList);
                this->completeFileListChannel.post(fileList);
                Logger::logInfo("Sending file list");
            }

            void sendRequestedFile(const std::string& relativeFilePath)
            {
                printf("Requested file %s\n", relativeFilePath.c_str());

                DataFile file;
                std::string path = this->filePath + std::string("/") + relativeFilePath;
                
                absl::Status status = this->loadDataFileFromPath(path, file);
                if(!status.ok())
                {
                    moduleError(status);
                    return;
                    // CLAID_THROW(Exception, "Error in DataSyncModule, file \"" << path << "\" was requested to be sent,\n"
                    // << "but it could not be loaded from the filesystem. Either the file does not exist or we do not have permission to read it.");
                }
                file.set_relative_path(relativeFilePath);
                this->dataFileChannel.post(file);
            }

            void onFileRequested(ChannelData<std::string> missingFileData)
            {
                printf("Requested file\n");   
                this->lastMessageFromFileReceiver = Time::now();
                sendRequestedFile(missingFileData.getData());
            }

            void onFileReceivalAcknowledged(ChannelData<std::string> receivedFile)
            {
                this->lastMessageFromFileReceiver = Time::now();
                if(this->deleteFileAfterSync)
                {
                    Logger::logInfo("Received acknowledgement of file received %s, deleting it.", receivedFile.getData().c_str());
                    std::string path = this->filePath + std::string("/") + receivedFile.getData();
                    Logger::logInfo("Deleting %s", path.c_str());
                    FileUtils::removeFileIfExists(path);
                }
            }

            size_t millisecondsSinceLastMessageFromFileReceiver()
            {
                size_t diff = std::chrono::duration_cast<std::chrono::milliseconds>((Time::now() - lastMessageFromFileReceiver)).count();
                
                return diff;
            }

            void periodicSync()
            {
                Logger::logInfo("PeriodicSyncCalled");
                // If a previous syncing process is still going on (e.g., file receiver is still requesting files),
                // do not start a new syncing process just yet.
                if(millisecondsSinceLastMessageFromFileReceiver() >= size_t(this->syncingPeriodInMs / 2))
                {
                    Logger::logInfo("!!!PERIODIC SYNC!!!");
                    sendFileList();
                }
            }

            // Get's called by the middleware once we connect to a remote server.
            void onConnectedToRemoteServer()
            {
                if(millisecondsSinceLastMessageFromFileReceiver() >= this->syncingPeriodInMs)
                {
                    // If we were not able to contact the DataReceiverModule for a while,
                    // we retry upon successfull connection to a remote server, assuming 
                    // that the DataReceiverModule is not running on the same host.
                    // If it is running on the same host, this code will never be executed,
                    // since we will always receive a timely response from the DataReceiverModule.
                    this->periodicSync();
                }
            }

            absl::Status loadDataFileFromPath(const std::string& path, DataFile& dataFile)
            {
                std::fstream file(path, std::ios::in | std::ios::binary);
                if(!file.is_open())
                {
                    return absl::InvalidArgumentError(absl::StrCat("Failed to load binary data from \"", path, "\".\n",
                        "Could not open File for reading."));
                }

                file.seekg(0, std::ios::end);
                int32_t numBytes = file.tellg();
                file.seekg(0, std::ios::beg);

                dataFile.mutable_file_data()->resize(numBytes);
                // Read the binary data into the data field
                if (!file.read(reinterpret_cast<char*>(dataFile.mutable_file_data()->data()), numBytes))
                {
                    return absl::InvalidArgumentError(
                        absl::StrCat("Failed to read binary data from file \"", path, "\"."));
                }

                return absl::OkStatus();
            }

        public:
      
      
            void initialize(Properties properties)
            {
                Logger::logInfo("DataSyncModule initialize");

                std::string what;
                std::string storagePath;
                std::string fileNameFormat;
                std::string fileType;


                properties.getStringProperty("filePath", this->filePath);
                properties.getNumberProperty("syncingPeriodInMs", this->syncingPeriodInMs);
                properties.getBoolProperty("deleteFileAfterSync", this->deleteFileAfterSync, false);

                if(properties.wasAnyPropertyUnknown())
                {
                    std::string unknownProperties;
                    properties.unknownPropertiesToString(unknownProperties);

                    this->moduleError(absl::StrCat("Missing properties: [", unknownProperties, "]. Please sepcify the properties in the configuration file."));
                    return;
                }

                if(this->getCommonDataPath() != "")
                {
                    claid::StringUtils::stringReplaceAll(this->filePath, "\%media_dir", this->getCommonDataPath());
                }
                else
                {
                    if(this->filePath.find("\%media_dir") != std::string::npos)
                    {
                        moduleError("Failed to initialize DataSyncModule. Storage path \"%s\" contains literal \%media_dir, \n"
                        "however media dir was never set. Make sure claid.setCommonDataPath() is called and a valid path is provided.");
                        return;
                    }
                }
                
           

                this->completeFileListChannelName = "FileSyncer/CompleteFileList";
                this->requestedFileChannelName = "FileSyncer/RequestedFileList";
                this->dataFileChannelName = "FileSyncer/DataFiles";
                this->receivedFilesAcknowledgementChannelName = "FileSyncer/ReceivedFilesAcknowledgement";

                // #ifdef __APPLE__
                //     #if TARGET_OS_IPHONE
                //         this->filePath = iOSApplicationPathHelper::getAppDocumentsPath() + std::string("/") + this->filePath;
                //     #endif
                // #endif

                this->completeFileListChannel = this->publish<std::vector<std::string>>(this->completeFileListChannelName);
                this->requestedFileChannel = this->subscribe<std::string>(this->requestedFileChannelName, &DataSyncModule::onFileRequested, this);
                this->dataFileChannel = this->publish<DataFile>(this->dataFileChannelName);

                 
                this->receivedFileAcknowledgementChannel = this->subscribe<std::string>(this->receivedFilesAcknowledgementChannelName, &DataSyncModule::onFileReceivalAcknowledged, this);
                
                this->lastMessageFromFileReceiver = Time::now();
                this->registerPeriodicFunction("PeriodicSyncFunction", &DataSyncModule::periodicSync, this, Duration::milliseconds(this->syncingPeriodInMs));
                this->periodicSync();
                Logger::logInfo("DataSyncModule done");

            }

       

             

    };
}