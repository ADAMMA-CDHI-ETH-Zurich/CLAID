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

#include "dispatch/core/DataCollection/DataSaver/DataSaverModule.hh"
#include "dispatch/core/Utilities/FileUtils.hh"
namespace claid
{
    void FileSaver::initialize(const std::map<std::string, std::string>& properties, DataSaverModule* parentModule)
    {  

        

        this->parentModule = parentModule;
        this->dataChannel = this->parentModule->subscribe<Untyped>(this->what, &FileSaver::onData, this);

        std::string channelName = this->what;
        claid::StringUtils::stringReplaceAll(channelName, "/", "_");
        claid::StringUtils::stringReplaceAll(this->storagePath, "\%channel_name", channelName);

        this->createStorageFolder(Path(""));
        this->createTmpFolderIfRequired(Path(""));

    }

    void FileSaver::onData(ChannelData<Untyped> data)
    {
        this->storeData(data);
    }

    void FileSaver::storeData(ChannelData<Untyped>& data)
    {
        std::string dataTypeName = this->dataChannel.getChannelDataTypeName();
        std::string reflectorName = this->serializer->getReflectorName();

        data.applySerializerToData(this->serializer, !this->excludeHeader);

       
        std::string pathStr = this->fileNameFormat;
        // This has to be done BEFORE calling strftime! Otherwise strftime will throw an exception, 
        // if any of custom %identifier values are present.
        claid::StringUtils::stringReplaceAll(pathStr, "\%sequence_id", std::to_string(data.getSequenceID()));
        claid::StringUtils::stringReplaceAll(pathStr, "\%timestamp", std::to_string(data.getTimestamp().toUnixTimestampMilliseconds()));
       
        pathStr = data.getTimestamp().strftime(pathStr.c_str());

        Path path(pathStr);

        uint64_t timestampMs = data.getTimestamp().toUnixTimestampMilliseconds();


            
        if(path != this->currentPath)
        {
            Logger::printfln("FileSaver::storeData changing file Timestamp %s %s %s %s", std::to_string(timestampMs).c_str(), pathStr.c_str(), currentPath.toString().c_str(), this->what.c_str());
            this->currentPath = path;
            beginNewFile(this->currentPath);
        }

        bool append = true;
        this->serializer->writeDataToFile(this->getCurrentFilePath(), this->currentFile);
    }

    void FileSaver::beginNewFile(const Path& path)
    {
        this->createStorageFolder(path);
        this->createTmpFolderIfRequired(path);

        if(this->tmpStoragePath != "")
        {
            this->moveTemporaryFilesToStorageDestination();
            this->currentFilePath = Path::join(this->tmpStoragePath, path).toString();
        }
        else
        {
            this->currentFilePath = (Path::join(this->storagePath, path).toString());
        }

        if(this->currentFile.is_open())
        {
            this->currentFile.flush();
            this->currentFile.close();
        }
        this->currentFile = std::ofstream(this->currentFilePath, std::ios::app);
        if(!this->currentFile.is_open())
        {
            CLAID_THROW(claid::Exception, "FileSaver: Failed to open file \" " << this->currentFilePath << "\"");
        }

        bool append = true;
        this->serializer->writeHeaderToFile(this->getCurrentFilePath(), this->currentFile);
    }

    std::string FileSaver::getCurrentFilePath()
    {
        return this->currentFilePath;
    }

    void FileSaver::storeDataHeader(const Path& path)
    {
        bool append = true;
        this->serializer->writeHeaderToFile(path, this->currentFile);
    }

    void FileSaver::getCurrentPathRelativeToStorageFolder(Path& path, const Time timestamp)
    {
        Time time = timestamp;
        path = Path(time.strftime(this->fileNameFormat.c_str()));
        //path = Path::join(this->storagePath, path.toString());
    }
     
    void FileSaver::createDirectoriesRecursively(const std::string& path)
    {
        if(FileUtils::dirExists(path))
        {
            return;
        }
        if(!FileUtils::createDirectoriesRecursively(path))
        {
            CLAID_THROW(Exception, "Error in FileSaver of DataSaverModule: Failed to create one or more directories of the following path: " << path);
        }
    }

    void FileSaver::createStorageFolder(const Path& currentSavePath)
    {
        std::string folderPath = Path::join(this->storagePath, currentSavePath.getFolderPath()).toString();
        this->createDirectoriesRecursively(folderPath);
    }

    void FileSaver::createTmpFolderIfRequired(const Path& currentSavePath)
    {
        if(this->tmpStoragePath == "")
        {
            return;
        }
        std::string folderPath = Path::join(this->tmpStoragePath, currentSavePath.getFolderPath()).toString();
        this->createDirectoriesRecursively(folderPath);
    }

    // In some cases, it might be benefical to first store files in a temporary location,
    // and only moving them to the final storage destination when writing to the file has been finished.
    // The user can choose to do that by specifying tmpFolderPath.
    // Whenever a new file is opened, all files in the tmp folder will be moved to their actual destination.
    void FileSaver::moveTemporaryFilesToStorageDestination()
    {
        std::vector<std::string> files;
        FileUtils::getAllFilesInDirectoryRecursively(this->tmpStoragePath, files);

        Logger::printfln("querying tmp folder %s\n", tmpStoragePath.c_str());
        for(const std::string& filePath : files)
        {
            Path path(filePath);
            
            std::string relativePath = path.getPathRelativeTo(this->tmpStoragePath);

            // Don't move the currently active file.
            Logger::printfln("Current path : %s Relative path: %s", currentPath.toString().c_str(), relativePath.c_str());
            if(relativePath == this->currentPath.toString())
            {
                Logger::printfln("Current path is relative path, not moving.");
                continue;
            }
            // If a file already exists at storage path (which it shoudln't!), then it will not be overriden but appended.
            if(!FileUtils::moveFileTo(Path::join(this->tmpStoragePath, relativePath), Path::join(this->storagePath, relativePath), true))
            {
                Logger::printfln("Moving file %s to %s failed.", Path::join(this->tmpStoragePath, relativePath).toString().c_str(), Path::join(this->storagePath, relativePath).toString().c_str());
            }
        }
    }
}