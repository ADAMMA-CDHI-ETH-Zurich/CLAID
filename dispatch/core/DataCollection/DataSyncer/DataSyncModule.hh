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
#include "dispatch/proto/sensor_data_types.pb.h"
#include "dispatch/proto/claidservice.pb.h"
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
                annotator.describePublishChannel<DataSyncPackage>("ToReceiverModuleChannel", "Channel to send data to the DataReceiverModule. Data might include the list of available files as well as the files themselves.");
                annotator.describeSubscribeChannel<DataSyncPackage>("FromReceiverModuleChannel", "Channel to receive data from the DataReceiverModule. Data might include requested files and acknowledgements.");
            }

        
        private:
            // On this channel, we post all the files available
            Channel<DataSyncPackage> toReceiverModuleChannel;

            // On this channel, the FileReceiver can request files that we will send.
            Channel<DataSyncPackage> fromReceiverModuleChannel;

            std::string filePath;

            Time lastMessageFromFileReceiver;
        
            // Constants
            static constexpr int SYNC_TIMEOUT_IN_MS = 3000;

         
            bool deleteFileAfterSync;
            bool onlyWhenCharging = false;
            bool requiresConnectionToRemoteServer = true;
            bool wasConnectedToRemoteServerDuringLastSync = false;
            int remoteServerConnectionWaitTimeSeconds = 60;

      
            Schedule syncingSchedule;
            std::vector<std::string> syncFunctionSchedulingNames;
            
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
                }

                moduleInfo(absl::StrCat("Scanning directory \"", this->filePath, "\" for files."));

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
                moduleInfo(absl::StrCat("Sending file list containing ", descriptors->descriptors_size(), " files"));
            }

            void sendRequestedFile(const DataSyncFileDescriptor& descriptor, const std::string& userId)
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
                this->toReceiverModuleChannel.postToUser(dataSyncPackage, userId);
            }

            void onPackageFromDataReceiver(ChannelData<DataSyncPackage> data)
            {
                const DataSyncPackage& pkg = data.getData();
                if(pkg.package_type() == DataSyncPackageType::REQUESTED_FILES_LIST)
                {
                    this->onFileRequested(pkg.file_descriptors(), data.getUserId());
                }
                else if(pkg.package_type() == DataSyncPackageType::ACKNOWLEDGED_FILES)
                {
                    this->onFileReceivalAcknowledged(pkg.file_descriptors());
                }
            }

            void onFileRequested(const DataSyncFileDescriptorList& requestedFiles, const std::string& userId)
            {
                printf("Requested file\n");   
                this->lastMessageFromFileReceiver = Time::now();

                for(const DataSyncFileDescriptor& fileDescriptor : requestedFiles.descriptors())
                {
                    sendRequestedFile(fileDescriptor, userId);
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
                startSync();
            }

            void startSync()
            {
                moduleInfo("Start sync called");
                if(onlyWhenCharging)
                {                
                    moduleInfo("Start sync called 1");
                    if(deviceInfoGatherer->isAvailable())
                    {
                                        moduleInfo("Start sync called 2");

                        if(deviceInfoGatherer->getDeviceChargingState() != DeviceChargingState::DEVICE_CHARGING_STATE_CHARGING)
                        {
                                            moduleInfo("Start sync called 3");

                            moduleWarning("Not charging, skipping sync this time");
                            return;
                        }
                                        moduleInfo("Start sync called 4");

                    }
                                    moduleInfo("Start sync called 5");

                }
                                moduleInfo("Start sync called 6");

                if(requiresConnectionToRemoteServer)
                {
                                    moduleInfo("Start sync called 7");

                    if(!isConnectedToRemoteServer())
                    {
                                        moduleInfo("Start sync called 8");

                        if(!waitUntilConnectedToRemoteServer(Duration::seconds(this->remoteServerConnectionWaitTimeSeconds)))
                        {
                                            moduleInfo("Start sync called 9");

                            wasConnectedToRemoteServerDuringLastSync = false;
                            moduleWarning("Not connected to remote server. Skipping synchronization and waiting for connection to come back.");
                            return;
                        }
                                        moduleInfo("Start sync called 10");

                    }
                                    moduleInfo("Start sync called 11");

                    wasConnectedToRemoteServerDuringLastSync = true;
                }
                                moduleInfo("Start sync called 12");

                // If a previous syncing process is still going on (e.g., file receiver is still requesting files),
                // do not start a new syncing process just yet.
                if(millisecondsSinceLastMessageFromFileReceiver() >= SYNC_TIMEOUT_IN_MS)
                {
                                    moduleInfo("Start sync called 13");

                    moduleInfo("DataSyncModule periodic sync 7");

                    Logger::logInfo("!!!PERIODIC SYNC!!!");
                    sendFileList();
                }
                else
                {
                                    moduleInfo("Start sync called 14");

                    moduleWarning("Synchronizing skipped as previous sync is still ongoing (milli seconds since last mesasge from file receiver < SYNC_TIMEOUT_IN_MS (3 seconds)");
                }
            }

            void setSyncingSchedule(Schedule schedule)
            {
                this->syncingSchedule = schedule;
                this->unregisterSyncFunction();
                this->registerSyncFunctionBasedOnSchedule();
            }

            // Get's called by the middleware once we connect to a remote server.
            void onConnectedToRemoteServer()
            {
                if(requiresConnectionToRemoteServer && !wasConnectedToRemoteServerDuringLastSync)
                {
                    // If we were not able to contact the DataReceiverModule on the Remote Server for a while,
                    // we retry upon successfull connection to a remote server, assuming 
                    // that the DataReceiverModule is not running on the same host.
                    // If it is running on the same host, the property requiresConnectionToRemoteServer should be disabled.
                    this->startSync();
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
                
                properties.getObjectProperty("syncingSchedule", this->syncingSchedule);
                properties.getBoolProperty("deleteFileAfterSync", this->deleteFileAfterSync, false);
                // If true, this Module requires that isConnectedToRemoteServer() is true when a syncing is due.
                // If isConnectedToRemoteServer() is not true, the Module will wait for a certain number of seconds (see below), and if not give up.
                properties.getBoolProperty("requiresConnectionToRemoteServer", this->requiresConnectionToRemoteServer);
                // If true, then syncing only happens when charging.
                // Only relevant on platforms where the charging state can be queried (e.g., Android). Not relevant for PCs/Servers (will simply be ignored).
                properties.getBoolProperty("onlyWhenCharging", this->onlyWhenCharging, false);
                // When a syncing is due, we require a connection to the remote server, and we are currently not connected,
                // then we wait a certain number of seconds for a connection to the remote server to be established.
                properties.getNumberProperty("remoteServerConnectionWaitTimeSeconds", this->remoteServerConnectionWaitTimeSeconds, 60);
                if(properties.wasAnyPropertyUnknown())
                {
                    std::string unknownProperties;
                    properties.unknownPropertiesToString(unknownProperties);

                    this->moduleFatal(absl::StrCat("Missing properties: [", unknownProperties, "]. Please sepcify the properties in the configuration file."));
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
                        moduleFatal("Failed to initialize DataSyncModule. Storage path \"%s\" contains literal \%media_dir, \n"
                        "however media dir was never set. Make sure claid.setCommonDataPath() is called and a valid path is provided.");
                        return;
                    }
                }
                
                // Exposing the internal "startSync" function, allowing other Modules to
                // trigger the synchronization via a remote function call.
                registerRemoteFunction("start_sync", &DataSyncModule::startSync, this);

                // Exposing the internal "setSyncingSchedule" function, allowing other Modules to
                // change the syncing schedule via a remote function call.
                registerRemoteFunction("set_syncing_schedule", &DataSyncModule::setSyncingSchedule, this);

                // #ifdef __APPLE__
                //     #if TARGET_OS_IPHONE
                //         this->filePath = iOSApplicationPathHelper::getAppDocumentsPath() + std::string("/") + this->filePath;
                //     #endif
                // #endif

                this->toReceiverModuleChannel = this->publish<DataSyncPackage>("ToDataReceiverModuleChannel");
                this->fromReceiverModuleChannel = this->subscribe<DataSyncPackage>("FromDataReceiverModuleChannel", &DataSyncModule::onPackageFromDataReceiver, this);
                               
                this->lastMessageFromFileReceiver = Time::now();
                
                registerSyncFunctionBasedOnSchedule();

                Logger::logInfo("DataSyncModule init done");

            }

            void unregisterSyncFunction()
            {
                for(const std::string& function : this->syncFunctionSchedulingNames)
                {
                    Logger::logInfo("Unregistering scheduled function %s",function.c_str());
                    this->unregisterPeriodicFunction(function);
                }
                Logger::logInfo("Scheduled functions are now: %d", this->syncFunctionSchedulingNames.size());
                this->syncFunctionSchedulingNames.clear();
            }

            void registerSyncFunctionBasedOnSchedule()
            {
                this->syncFunctionSchedulingNames = this->registerFunctionBasedOnSchedule(
                    "PeriodicSyncFunction", 
                    this->syncingSchedule,
                    &DataSyncModule::periodicSync, 
                    this
                );
            }

       

             

    };
}