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
#include "dispatch/core/Module/Module.hh"
#include "dispatch/core/Utilities/FileUtils.hh"
#include "dispatch/core/Utilities/StringUtils.hh"
#include "dispatch/proto/sensor_data_types.grpc.pb.h"
#ifdef __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
    #include "CollectorAPI/iOSHelper/iOSApplicationPathHelper.hpp"
    #endif
#endif

#include "absl/strings/str_split.h"
#include "absl/status/status.h"
#include <queue>
namespace claid
{
    // Counterpart to DataSyncModule.
    // Receives a list of files and compares it with files available in a specified directory.
    // All files that are missing are requested from the DataSyncModule, which will then be received
    // and stored in the path.
    class DataReceiverModule : public Module
    {

        private:

            // On this channel, we receive all the files available
            Channel<std::vector<std::string>> completeFileListChannel;

            // On this channel, we post files that are missing.
            // They will be send by the DataSyncModule.
            Channel<std::string> requestedFileChannel;

            // On this channel, we receive files from the DataSyncModule.
            Channel<DataFile> dataFileChannel;

            // On this channel, we acknowledge received files.
            // If "deleteFilesAfterSync" is activated in DataSyncModule, it will delete the file locally.
            Channel<std::string> receivedFilesAcknowledgementChannel;

            std::string filePath;
            std::string completeFileListChannelName;
            std::string requestedFileChannelName;
            std::string dataFileChannelName;
            std::string receivedFilesAcknowledgementChannelName;

            // bool storeArrivalTimePerFile = false;

            std::queue<std::string> missingFilesQueue;

            template<typename T>
            bool isElementContainedInVector(const std::vector<T>& vector, const T& element)
            {
                return std::find(vector.begin(), vector.end(), element) != vector.end();
            }

            // Get elements that are contained in both lists.
            void getMissingElements(
                    const std::vector<std::string>& completeList, 
                    std::vector<std::string>& missingList)
            {
                missingList.clear();
                std::string path;
                for(const std::string& fileName : completeList)
                {   
                    path = this->filePath + std::string("/") + fileName;
                    if(!FileUtils::fileExists(path))
                    {
                        missingList.push_back(fileName);
                    }
                }
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


            // Splits a/b/c/d.txt into a/b/c and d.txt
            void splitPathIntoFolderAndFileName(const std::string& path, std::string& folderPath, std::string& fileName)
            {
                if(path.find("/") == std::string::npos)
                {
                    // No folder, assume path is only a fileName.
                    folderPath = "";
                    fileName = path;
                    return;
                }
                int index = path.size() - 1;
                while(index > 0)
                {
                    if(path[index] == '/')
                    {
                        folderPath = path.substr(0, index);
                        fileName = path.substr(index + 1, path.size());
                        return;
                    }
                    index--;
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

            void onCompleteFileListReceived(ChannelData<std::vector<std::string>> data)
            {
                const std::vector<std::string>& completeList = data.getData();
                Logger::logInfo("Received complete file list");

                for(const std::string& value : completeList)
                {
                    Logger::logInfo("Complete file %s", value.c_str());
                }                
 
                std::vector<std::string> missingFiles;
                getMissingElements(completeList, missingFiles);

                this->missingFilesQueue = std::queue<std::string>();

                for(const std::string& value : missingFiles)
                {
                    this->missingFilesQueue.push(value);
                    Logger::logInfo("Missing %s", value.c_str());
                }
                
                this->requestNextFile();
            }

            void requestNextFile()
            {
                printf("Requesting next file %d\n", this->missingFilesQueue.empty());
                if(this->missingFilesQueue.empty())
                {
                    return;
                }

                std::string file = this->missingFilesQueue.front();
                printf("Requesting file %s\n", file.c_str());
                this->missingFilesQueue.pop();
                this->requestedFileChannel.post(file);
            }

            void onDataFileReceived(ChannelData<DataFile> data)
            {
                std::cout << "Received data file " << data.getData().relative_path() << "\n";

                this->requestNextFile();
                const DataFile& dataFile = data.getData();

                const std::string& relativePath = dataFile.relative_path();
                std::string folderPath;
                std::string filePath;
                splitPathIntoFolderAndFileName(relativePath, folderPath, filePath);
                printf("folder file %s %s\n", folderPath.c_str(), filePath.c_str());
                 
                if(folderPath != "")
                {
                    std::string targetFolderPath = this->filePath + std::string("/") + folderPath;
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
                    this->filePath + std::string("/") + folderPath + std::string("/") + filePath;
                
                if(!saveDataFileToPath(dataFile, targetFilePath))
                {
                    return;
                }

                // if(this->storeArrivalTimePerFile)
                // {
                //     Time arrivalTime = Time::now();
                //     std::string arrivalTimeStampPath = 
                //         this->filePath + std::string("/") + folderPath + std::string("/") + filePath + std::string("_arrival.xml");
                    
                //     XMLSerializer serializer;
                //     serializer.serialize("Data", arrivalTime);

                //     XMLDocument xmlDocument(serializer.getXMLNode());
                //     xmlDocument.saveToFile(arrivalTimeStampPath);
                // }

                // Send acknowledgement.
                this->receivedFilesAcknowledgementChannel.post(dataFile.relative_path());
            }

            bool setupStorageFolder()
            {
                #ifdef __APPLE__
                    #if TARGET_OS_IPHONE
                        this->filePath = iOSApplicationPathHelper::getAppDocumentsPath() + std::string("/") + this->filePath;
                    #endif
                #endif
                
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

            void initialize(const std::map<std::string, std::string>& propertiesMap)
            {
                Logger::logInfo("DataReceiverModule initialize");
                PropertyHelper properties(propertiesMap);

                properties.getProperty("filePath", this->filePath);

                completeFileListChannelName = "FileSyncer/CompleteFileList";
                requestedFileChannelName = "FileSyncer/RequestedFileList";
                dataFileChannelName = "FileSyncer/DataFiles";
                receivedFilesAcknowledgementChannelName = "FileSyncer/ReceivedFilesAcknowledgement";

                // Create output directory, if not exists.
                if(!this->setupStorageFolder())
                {
                    return;
                }
                // Check if we can access output directory, scan for existing files.
                // Throws exception if we cannot access the directory.
                std::vector<std::string> output;
                getListOfFilesInTargetDirectory(output);

                this->completeFileListChannel = this->subscribe<std::vector<std::string>>(completeFileListChannelName, &DataReceiverModule::onCompleteFileListReceived, this);
                this->requestedFileChannel = this->publish<std::string>(requestedFileChannelName);
                this->dataFileChannel = this->subscribe<DataFile>(dataFileChannelName, &DataReceiverModule::onDataFileReceived, this);
                this->receivedFilesAcknowledgementChannel = this->publish<std::string>(receivedFilesAcknowledgementChannelName);
                Logger::logInfo("DataReceiverModule initialize done");

            }

            bool saveDataFileToPath(const DataFile& dataFile, const std::string& path)
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

        public:

     


    };
}