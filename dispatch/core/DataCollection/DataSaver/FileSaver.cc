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

#include "dispatch/core/DataCollection/DataSaver/DataSaverModule.hh"
#include "dispatch/core/Utilities/FileUtils.hh"
#include "dispatch/core/DataCollection/Serializer/DataSerializerFactory.hh"

namespace claid
{
    FileSaver::FileSaver()
    {

    }

    absl::Status FileSaver::initialize(const std::string& what, const std::string& storagePath, 
        const std::string& fileNameFomat, const std::string& fileType, bool overrideExistingFiles = false,
        std::string defaultMediaPath = "")
    {  
        if(this->initialized)
        {
            return absl::InvalidArgumentError("FileSaver: Initialize was called twice");
        }

        this->what = what;
        this->storagePath = storagePath;
        this->fileNameFormat = fileNameFomat;
        this->fileType = fileType;
        this->overrideExistingFiles = overrideExistingFiles;
        this->defaultMediaPath = defaultMediaPath;

        if(this->storagePath.size() > 0 && this->storagePath[this->storagePath.size() - 1] != '/')
        {
            this->storagePath += "/";
        }

        Logger::logInfo("media dir %s", this->defaultMediaPath.c_str());
        if(this->defaultMediaPath != "")
        {
            claid::StringUtils::stringReplaceAll(this->storagePath, "\%media_dir", this->defaultMediaPath);
        }
        else
        {
            if(this->storagePath.find("\%media_dir") != std::string::npos)
            {
                return absl::NotFoundError("Failed to initialize FileSaver. Storage path \"%s\" contains literal \%media_dir, \n"
                "however media dir was never set. Make sure claid.setCommonDataPath() is called and a valid path is provided.");
            }
        }
       
        this->serializer = DataSerializerFactory::getInstance()->getSerializerForDataType(fileType);
        if(this->serializer == nullptr)
        {
            return absl::NotFoundError(absl::StrCat("Unable to find serializer for data type \"", fileType, "\"."));
        }

        this->serializer->setOverrideExistingFiles(overrideExistingFiles);

        std::string channelName = this->what;
        claid::StringUtils::stringReplaceAll(channelName, "/", "_");
        // claid::StringUtils::stringReplaceAll(this->storagePath, "\%channel_name", channelName);
        

        absl::Status status = this->createStorageFolder(Path(this->storagePath));
        if(!status.ok())
        {
            return status;
        }

        status = this->createTmpFolderIfRequired(Path(""));
        if(!status.ok())
        {
            return status;
        }

        this->initialized = true;

        return absl::OkStatus();
    }

    absl::Status FileSaver::onNewData(ChannelData<google::protobuf::Message>& data)
    {
        return FileSaver::onNewData(data.getDataAsPtr(), data.getTimestamp());
    }

    absl::Status FileSaver::onNewData(std::shared_ptr<const google::protobuf::Message> data, const Time& timestamp)
    {       
        Logger::logInfo("FileSaver on new data");
        std::string pathStr = this->fileNameFormat;
        // This has to be done BEFORE calling strftime! Otherwise strftime will throw an exception, 
        // if any of custom %identifier values are present.
       // claid::StringUtils::stringReplaceAll(pathStr, "\%sequence_id", std::to_string(data.getSequenceID()));
        claid::StringUtils::stringReplaceAll(pathStr, "\%timestamp", std::to_string(timestamp.toUnixTimestampMilliseconds()));
       
        if(this->defaultMediaPath != "")
        {
            claid::StringUtils::stringReplaceAll(pathStr, "\%media_dir", this->defaultMediaPath);
        }

        pathStr = timestamp.strftime(pathStr.c_str());

        Path path(pathStr);
        path = Path::join(this->storagePath, path);

        uint64_t timestampMs = timestamp.toUnixTimestampMilliseconds();


        absl::Status status;
        if(path != this->currentPath)
        {
            this->currentPath = path;
            Logger::logInfo("FileSaver::storeData changing file Timestamp %s %s %s %s", std::to_string(timestampMs).c_str(), pathStr.c_str(), currentPath.toString().c_str(), this->what.c_str());
            status = beginNewFile(this->currentPath);
            if(!status.ok())
            {
                return status;
            }
        }

        status = this->serializer->onNewData(data);
        hasReceivedData = true;
        return status;
    }

    absl::Status FileSaver::beginNewFile(const Path& path)
    {
        Logger::logInfo("Beginning new file");
        absl::Status status = this->createStorageFolder(path);
        if(!status.ok())
        {
            return status;
        }

        status = this->createTmpFolderIfRequired(path);
        if(!status.ok())
        {
            return status;
        }

        if(this->tmpStoragePath != "")
        {
            this->moveTemporaryFilesToStorageDestination();
            this->currentFilePath = Path::join(this->tmpStoragePath, path).toString();
        }
        else
        {
            this->currentFilePath = (Path::join(this->storagePath, path).toString());
        }

        if(hasReceivedData)
        {
            status = this->serializer->finishFile();
            if(!status.ok())
            {
                return status;
            }
        }
        

        Logger::logInfo("PATH %s", path.toString().c_str());
        status = this->serializer->beginNewFile(path.toString());
        if(!status.ok())
        {
            return status;
        }


        return absl::OkStatus();
    }

    std::string FileSaver::getCurrentFilePath()
    {
        return this->currentFilePath;
    }

    // void FileSaver::storeDataHeader(const Path& path)
    // {
    //     bool append = true;
    //     this->serializer->writeHeaderToFile(path, this->currentFile);
    // }

    void FileSaver::getCurrentPathRelativeToStorageFolder(Path& path, const Time timestamp)
    {
        Time time = timestamp;
        path = Path(time.strftime(this->fileNameFormat.c_str()));
        //path = Path::join(this->storagePath, path.toString());
    }
     
    absl::Status FileSaver::createDirectoriesRecursively(const std::string& path)
    {
        if(FileUtils::dirExists(path))
        {
            return absl::OkStatus();
        }
        if(!FileUtils::createDirectoriesRecursively(path))
        {
            return absl::UnavailableError(absl::StrCat("Error in FileSaver of DataSaverModule: Failed to create one or more directories of the following path: \"", path, "\""));
        }

        return absl::OkStatus();
    }

    absl::Status FileSaver::createStorageFolder(const Path& currentSavePath)
    {
        Logger::logInfo("Getting currentSavePath: %s %s", currentSavePath.toString().c_str(), currentSavePath.getFolderPath().c_str());
        std::string folderPath = currentSavePath.getFolderPath();
        return this->createDirectoriesRecursively(folderPath);
    }

    absl::Status FileSaver::createTmpFolderIfRequired(const Path& currentSavePath)
    {
        if(this->tmpStoragePath == "")
        {
            return absl::OkStatus();
        }
        std::string folderPath = Path::join(this->tmpStoragePath, currentSavePath.getFolderPath()).toString();
        return this->createDirectoriesRecursively(folderPath);
    }

    // In some cases, it might be benefical to first store files in a temporary location,
    // and only moving them to the final storage destination when writing to the file has been finished.
    // The user can choose to do that by specifying tmpFolderPath.
    // Whenever a new file is opened, all files in the tmp folder will be moved to their actual destination.
    void FileSaver::moveTemporaryFilesToStorageDestination()
    {
        std::vector<std::string> files;
        FileUtils::getAllFilesInDirectoryRecursively(this->tmpStoragePath, files);

        Logger::logInfo("querying tmp folder %s\n", tmpStoragePath.c_str());
        for(const std::string& filePath : files)
        {
            Path path(filePath);
            
            std::string relativePath = path.getPathRelativeTo(this->tmpStoragePath);

            // Don't move the currently active file.
            Logger::logInfo("Current path : %s Relative path: %s", currentPath.toString().c_str(), relativePath.c_str());
            if(relativePath == this->currentPath.toString())
            {
                Logger::logInfo("Current path is relative path, not moving.");
                continue;
            }
            // If a file already exists at storage path (which it shoudln't!), then it will not be overriden but appended.
            if(!FileUtils::moveFileTo(Path::join(this->tmpStoragePath, relativePath), Path::join(this->storagePath, relativePath), true))
            {
                Logger::logInfo("Moving file %s to %s failed.", Path::join(this->tmpStoragePath, relativePath).toString().c_str(), Path::join(this->storagePath, relativePath).toString().c_str());
            }
        }
    }

    absl::Status FileSaver::endFileSaving()
    {
        if(!this->initialized)
        {
            return absl::InvalidArgumentError("FileSaver: Failed to end file saving. Initialize was not called.");
        }

        absl::Status status = this->serializer->finishFile();
        if(!status.ok())
        {
            return status;
        }

        this->initialized = false;


        return absl::OkStatus();
    }

   
}