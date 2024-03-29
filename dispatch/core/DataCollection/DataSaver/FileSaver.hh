#pragma once

#include "dispatch/core/DataCollection/Serializer/DataSerializer.hh"
#include "dispatch/core/Utilities/Path.hh"
#include "dispatch/core/Module/ChannelData.hh"

#include <fstream>

namespace claid
{
    class DataSaverModule;

    class FileSaver
    {
        private:
            std::shared_ptr<DataSerializer> serializer;
            std::string what;
            std::string storagePath;
            std::string fileNameFormat;
            std::string fileType;

            
            std::string tmpStoragePath;
            std::string currentFilePath;

            std::string defaultMediaPath = "";


            Path currentPath;

            std::string getCurrentFilePath();
            void getCurrentPathRelativeToStorageFolder(Path& path, const Time timestamp);
            absl::Status createDirectoriesRecursively(const std::string& path);
            absl::Status beginNewFile(const Path& path);
            

            void moveTemporaryFilesToStorageDestination();

            absl::Status createStorageFolder(const Path& currentSavePath);
            absl::Status createTmpFolderIfRequired(const Path& currentSavePath);

            bool initialized = false;
            bool hasReceivedData = false;
            bool overrideExistingFiles = false;

        public:

            // template<typename Reflector>
            // void reflect(Reflector& reflector)
            // {
            // reflector.member("serializer", serializer, "");

            // }
   

            

            FileSaver();

            absl::Status initialize(const std::string& what, const std::string& storagePath, 
                const std::string& fileNameFomat, const std::string& fileType, bool overrideExistingFiles,
                std::string defaultMediaPath);
            absl::Status onNewData(ChannelData<google::protobuf::Message>& data);
            absl::Status onNewData(std::shared_ptr<const google::protobuf::Message> data, const Time& timestamp);

            absl::Status endFileSaving();


            // void storeDataHeader(const Path& path);
                
            

    };
}