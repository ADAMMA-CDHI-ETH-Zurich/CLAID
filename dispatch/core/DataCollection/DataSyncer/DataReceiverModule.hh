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
#include "dispatch/core/Utilities/Path.hh"
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
#include <queue>
#include <map>

using namespace claidservice;

namespace claid
{
    // Counterpart to DataSyncModule.
    // Receives a list of files and compares it with files available in a specified directory.
    // All files that are missing are requested from the DataSyncModule, which will then be received
    // and stored in the path.
    class DataReceiverModule : public Module
    {
        public:
            static void annotateModule(ModuleAnnotator& annotator)
            {
                annotator.setModuleCategory("DataCollection");
                annotator.setModuleDescription(absl::StrCat(
                    "The DataReceiverModule is the counterpart to the DataSyncModule. Together, the two Modules allow to synchronize files stored on the file system between different hosts.\n"
                    "Files can be synchronized based on a synchronization intervall (for example, every hour). Each file is tagged with the original host and user name, allowing you to\n"
                    "receive data from many different hosts and users and store it in separate folders."
                ));
                annotator.describePublishChannel<DataSyncPackage>("ToSyncModuleChannel", "Channel to send data to the DataSyncModule. Data might include requested files and acknowledgements.");
                annotator.describeSubscribeChannel<DataSyncPackage>("FromSyncModuleChannel", "Channel to receive data from the DataSyncModule. Data might include the list of available files as well as the files themselves.");
               
            }

        private:

            Channel<DataSyncPackage> fromDataSyncModuleChannel;
            Channel<DataSyncPackage> toDataSyncModuleChannel;

            std::string filePath;

            // bool storeArrivalTimePerFile = false;

            // One queue per user that has a DataSyncModule connected.
            // Indicates which files of the user we are missing.
            std::map<std::string, std::queue<std::string>> missingFilesQueuePerUser;

            template<typename T>
            bool isElementContainedInVector(const std::vector<T>& vector, const T& element)
            {
                return std::find(vector.begin(), vector.end(), element) != vector.end();
            }

            // Get elements that are contained in both lists.
            void getMissingFilesOfUser(
                    const std::string& userId, 
                    const DataSyncFileDescriptorList& descriptorList, 
                    std::vector<std::string>& missingList)
            {
                missingList.clear();
                std::string path;
                for(const DataSyncFileDescriptor& descriptor : descriptorList.descriptors())
                {   
                    const std::string& relativePath = descriptor.relative_file_path();
                    path = getUserStoragePath(userId) + std::string("/") + relativePath;
                    // If the file doesnt exist yet, we add it to the list of missing files.
                    if(!FileUtils::fileExists(path))
                    {
                        missingList.push_back(relativePath);
                        continue;
                    }

                    uint64_t fileSize;
                    if(!FileUtils::getFileSize(path, fileSize))
                    {
                        moduleError(absl::StrCat("Unable to get file size of file \"", path, "\""));
                        continue;
                    }

                    // If the file exists, but has a different file size, we also request it again.
                    // The current file will then be replaced.
                    if(fileSize != descriptor.file_size())
                    {
                        missingList.push_back(relativePath);
                        continue;
                    }
                }
            }

            std::string getUserStoragePath(const std::string& userId)
            {
                return this->filePath + "/" + userId;
            }

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

            void getListOfFilesInTargetDirectory(std::vector<std::string>& output)
            {
                output.clear();
                if(!FileUtils::getAllFilesInDirectoryRecursively(this->filePath, output))
                {
                    printf("Error in DataReceiverModule, cannot determine missing files.\n"
                        "Cannot retrieve list of files available in directory \"%s\".\n"
                        "Either the directory does not exist or we do not have permissions to access it.\n",
                        this->filePath.c_str());

                    return;
                }

                for(std::string& path : output)
                {
                    makePathRelative(path);
                }
            }

            void onDataFromDataSyncModule(ChannelData<DataSyncPackage> data)
            {
                const DataSyncPackage& pkg = data.getData();
                Logger::logInfo(
                    "DataReceiverModule received data from DataSyncModule, package type is: %s",
                     DataSyncPackageType_Name(pkg.package_type()).c_str()
                );
                if(pkg.package_type() == DataSyncPackageType::ALL_AVAILABLE_FILES_LIST)
                {
                    this->onCompleteFileListReceivedFromUser(pkg.file_descriptors(), data.getUserId());
                }
                else if(pkg.package_type() == DataSyncPackageType::FILES_DATA)
                {
                    this->onFileReceivedFromUser(pkg.file_descriptors(), data.getUserId());
                }
            }

            void onCompleteFileListReceivedFromUser(const DataSyncFileDescriptorList& descriptorList, const std::string& userId)
            {
                Logger::logInfo("Received complete file list from user %s", userId.c_str());
 
                std::vector<std::string> missingFiles;
                getMissingFilesOfUser(userId, descriptorList, missingFiles);

                this->missingFilesQueuePerUser[userId] = std::queue<std::string>();

                for(const std::string& value : missingFiles)
                {
                    this->missingFilesQueuePerUser[userId].push(value);
                    Logger::logInfo("Missing %s", value.c_str());
                }
                
                this->requestNextFileFromUser(userId);
            }

            void requestNextFileFromUser(const std::string& userId)
            {
                std::queue<std::string>& filesQueue = this->missingFilesQueuePerUser[userId]; 
                printf("Requesting next file %lu\n", filesQueue.size());
                if(filesQueue.empty())
                {
                    return;
                }

                std::string file = filesQueue.front();
                printf("Requesting file %s\n", file.c_str());
                filesQueue.pop();

                DataSyncPackage dataSyncPackage;
                dataSyncPackage.set_package_type(DataSyncPackageType::REQUESTED_FILES_LIST);

                DataSyncFileDescriptorList* descriptors = dataSyncPackage.mutable_file_descriptors();
                descriptors->add_descriptors()->set_relative_file_path(file);

                this->toDataSyncModuleChannel.postToUser(dataSyncPackage, userId);
            }

            void onFileReceivedFromUser(const DataSyncFileDescriptorList& descriptorList, const std::string& userId)
            {
                this->requestNextFileFromUser(userId);

                for(const DataSyncFileDescriptor& fileDescriptor : descriptorList.descriptors())
                {
                    const std::string& relativePath = fileDescriptor.relative_file_path();
                    std::string folderPath;
                    std::string filePath;
                    Path::splitPathIntoFolderAndFileName(relativePath, folderPath, filePath);
                    printf("folder file %s %s %s\n", relativePath.c_str(), folderPath.c_str(), filePath.c_str());
                    
                    if(folderPath != "")
                    {
                        std::string targetFolderPath = getUserStoragePath(userId) + std::string("/") + folderPath;
                        if(!FileUtils::dirExists(targetFolderPath))
                        {
                            if(!FileUtils::createDirectoriesRecursively(targetFolderPath))
                            {
                                moduleError(absl::StrCat("Error in DataReceiverModule, cannot create target folder \"", targetFolderPath, "\"."));
                                return;
                            }
                        }
                    }

                    std::string targetFilePath = 
                        getUserStoragePath(userId) + std::string("/") + folderPath + std::string("/") + filePath;
                    
                    if(!saveDataFileToPath(fileDescriptor, targetFilePath))
                    {
                        return;
                    }

                    // Send acknowledgement.
                    DataSyncPackage dataSyncPackage;
                    dataSyncPackage.set_package_type(DataSyncPackageType::ACKNOWLEDGED_FILES);

                    DataSyncFileDescriptorList* descriptors = dataSyncPackage.mutable_file_descriptors();
                    descriptors->add_descriptors()->set_relative_file_path(relativePath);
                    this->toDataSyncModuleChannel.postToUser(dataSyncPackage, userId);
                }
            }

            bool setupStorageFolder()
            {
                // #ifdef __APPLE__
                //     #if TARGET_OS_IPHONE
                //         this->filePath = iOSApplicationPathHelper::getAppDocumentsPath() + std::string("/") + this->filePath;
                //     #endif
                // #endif
                
                std::string savePath = this->filePath;

                if(!FileUtils::dirExists(savePath))
                {
                    if(!FileUtils::createDirectoriesRecursively(savePath))
                    {
                        moduleError(absl::StrCat("Error in DataReceiverModule. Cannot create save directory \"", savePath, "."));
                        return false;
                    }
                }
                return true;
            }

            void initialize(Properties properties)
            {
                Logger::logInfo("DataReceiverModule initialize");

                properties.getStringProperty("storagePath", this->filePath);

                // Create output directory, if not exists.
                if(!this->setupStorageFolder())
                {
                    return;
                }
                // Check if we can access output directory, scan for existing files.
                // Throws exception if we cannot access the directory.
                std::vector<std::string> output;
                getListOfFilesInTargetDirectory(output);

                this->toDataSyncModuleChannel = this->publish<DataSyncPackage>("ToDataSyncModuleChannel");
                this->fromDataSyncModuleChannel = this->subscribe<DataSyncPackage>("FromDataSyncModuleChannel", &DataReceiverModule::onDataFromDataSyncModule, this);

                Logger::logInfo("DataReceiverModule initialize done");

            }

            bool saveDataFileToPath(const DataSyncFileDescriptor& dataFile, const std::string& path)
            {
                std::fstream file(path, std::ios::out | std::ios::binary);
                if(!file.is_open())
                {
                    moduleError(absl::StrCat("Error, cannot save binary data to \"", path, "\".\n. Could not open File for writing."));
                    return false;
                }

                file.write(dataFile.file_data().data(), dataFile.file_data().size());
                return true;
            }    
    };
}