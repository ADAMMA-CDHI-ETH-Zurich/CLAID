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

using namespace claidservice;

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
                    "The DataSyncModule is the counterpart to the DataReceiverModule. Together, the two Modules allow to synchronize files stored on the file system between different hosts.\n"
                    "Files can be synchronized based on a synchronization intervall (for example, every hour). Each file is tagged with the original host and user name, allowing you to\n"
                    "receive data from many different hosts and users and store it in separate folders."
                ));               
            }

        
        private:
            // On this channel, we post all the files available
            Channel<DataSyncPackage> toReceiverModuleChannel;

            // On this channel, the FileReceiver can request files that we will send.
            Channel<DataSyncPackage> fromReceiverModuleChannel;

            std::string filePath;

            Time lastMessageFromFileReceiver;
        


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

            void buildFileList(DataSyncFileDescriptorList& descriptorList)
            {
                descriptorList.clear_descriptors();

                std::vector<std::string> fileList;
                if(!FileUtils::getAllFilesInDirectoryRecursively(this->filePath, fileList))
                {
                    moduleWarning(absl::StrCat("Unable scan directory \"", this->filePath, "\" for files."));
                    return;
                    //CLAID_THROW(Exception, "Error in DataSyncModule, cannot scan directory \"" << this->filePath << "\" "
                    //<< "for files. Directory either does not exist or we do not have reading permissions.");
                }

                for(std::string& path : fileList)
                {
                    uint64_t fileSize;
                    if(!FileUtils::getFileSize(path, fileSize))
                    {
                        moduleError(absl::StrCat("Failed to get size of file \"", path, "\""));
                        return;
                    }

                    makePathRelative(path);
                    DataSyncFileDescriptor* descriptor = descriptorList.add_descriptors();
                    descriptor->set_hash(0); // For now, we do not use any hashing.
                    descriptor->set_file_size(fileSize);
                    descriptor->set_relative_file_path(path);
                    // No data for now, as we are only sending a list of files.
                    // The actual data files will be sent once the DataReceiverModule tells us which files it wants.
                }

            }

            void sendFileList()
            {
                DataSyncPackage dataSyncPackage;
                dataSyncPackage.set_package_type(DataSyncPackageType::ALL_AVAILABLE_FILES_LIST);

                DataSyncFileDescriptorList* descriptors = dataSyncPackage.mutable_file_descriptors();
                this->buildFileList(*descriptors);
                this->toReceiverModuleChannel.post(dataSyncPackage);
                moduleInfo("Sending file list");
            }

            void sendRequestedFile(const DataSyncFileDescriptor& descriptor)
            {
                const std::string& relativeFilePath = descriptor.relative_file_path();
                printf("Requested file %s\n", relativeFilePath.c_str());

                DataSyncFileDescriptor fileToSend;
                uint64_t fileSize;
                std::string path = this->filePath + std::string("/") + relativeFilePath;
                
                absl::Status status = this->loadDataFileFromPath(path, fileToSend);
                if(!status.ok())
                {
                    moduleError(status);
                    return;
                    // CLAID_THROW(Exception, "Error in DataSyncModule, file \"" << path << "\" was requested to be sent,\n"
                    // << "but it could not be loaded from the filesystem. Either the file does not exist or we do not have permission to read it.");
                }
                if(!FileUtils::getFileSize(path, fileSize))
                {
                    moduleError(absl::StrCat("Failed to get size of file \"", path, "\", cannot send requested file."));
                    return;
                }
                
                fileToSend.set_relative_file_path(relativeFilePath);
                fileToSend.set_hash(0); // Currently not using any hash.
                // file size was already set in loadDataFileFromPath.

                DataSyncPackage dataSyncPackage;
                dataSyncPackage.set_package_type(DataSyncPackageType::FILES_DATA);

                DataSyncFileDescriptorList* descriptorList = dataSyncPackage.mutable_file_descriptors();
                *descriptorList->add_descriptors() = fileToSend;
                this->toReceiverModuleChannel.post(dataSyncPackage);
            }

            void onPackageFromDataReceiver(ChannelData<DataSyncPackage> data)
            {
                const DataSyncPackage& pkg = data.getData();
                if(pkg.package_type() == DataSyncPackageType::REQUESTED_FILES_LIST)
                {
                    this->onFileRequested(pkg.file_descriptors());
                }
                else if(pkg.package_type() == DataSyncPackageType::ACKNOWLEDGED_FILES)
                {
                    this->onFileReceivalAcknowledged(pkg.file_descriptors());
                }
            }

            void onFileRequested(const DataSyncFileDescriptorList& requestedFiles)
            {
                printf("Requested file\n");   
                this->lastMessageFromFileReceiver = Time::now();

                for(const DataSyncFileDescriptor& fileDescriptor : requestedFiles.descriptors())
                {
                    sendRequestedFile(fileDescriptor);
                }
                
            }

            void onFileReceivalAcknowledged(const DataSyncFileDescriptorList& acknowledgedFiles)
            {
                this->lastMessageFromFileReceiver = Time::now();
                for(const DataSyncFileDescriptor& fileDescriptor : acknowledgedFiles.descriptors())
                {
                    if(this->deleteFileAfterSync)
                    {
                        const std::string& relativePath = fileDescriptor.relative_file_path().c_str();
                        Logger::logInfo("Received acknowledgement of file received %s, deleting it.", fileDescriptor.relative_file_path().c_str());
                        std::string path = this->filePath + std::string("/") + relativePath;
                        Logger::logInfo("Deleting %s", path.c_str());
                        FileUtils::removeFileIfExists(path);
                    }
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
                if(millisecondsSinceLastMessageFromFileReceiver() >= size_t(this->syncingPeriodInMs))
                {
                    // If we were not able to contact the DataReceiverModule for a while,
                    // we retry upon successfull connection to a remote server, assuming 
                    // that the DataReceiverModule is not running on the same host.
                    // If it is running on the same host, this code will never be executed,
                    // since we will always receive a timely response from the DataReceiverModule.
                    this->periodicSync();
                }
            }

            absl::Status loadDataFileFromPath(const std::string& path, DataSyncFileDescriptor& dataFile)
            {
                std::fstream file(path, std::ios::in | std::ios::binary);
                if(!file.is_open())
                {
                    return absl::InvalidArgumentError(absl::StrCat("Failed to load binary data from \"", path, "\".\n",
                        "Could not open File for reading."));
                }

                file.seekg(0, std::ios::end);
                uint64_t numBytes = file.tellg();
                file.seekg(0, std::ios::beg);

                dataFile.mutable_file_data()->resize(numBytes);
                dataFile.set_file_size(numBytes);
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
                
           

            

                // #ifdef __APPLE__
                //     #if TARGET_OS_IPHONE
                //         this->filePath = iOSApplicationPathHelper::getAppDocumentsPath() + std::string("/") + this->filePath;
                //     #endif
                // #endif

                this->toReceiverModuleChannel = publish<DataSyncPackage>("ToReceiverChannel");
                this->fromReceiverModuleChannel = this->subscribe<DataSyncPackage>("FromReceiverChannel", &DataSyncModule::onPackageFromDataReceiver, this);
                               
                this->lastMessageFromFileReceiver = Time::now();
                this->registerPeriodicFunction("PeriodicSyncFunction", &DataSyncModule::periodicSync, this, Duration::milliseconds(this->syncingPeriodInMs));
                this->periodicSync();
                Logger::logInfo("DataSyncModule done");

            }

       

             

    };
}