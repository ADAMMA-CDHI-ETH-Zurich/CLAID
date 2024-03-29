#include "dispatch/core/DataCollection/DataSaver/DataSaverModule.hh"
namespace claid
{
    
    
    void DataSaverModule::initialize(Properties properties)
    {

        std::string storagePath;
        std::string fileNameFormat;
        std::string fileType;
        bool overrideExistingFiles;


        properties.getStringProperty("storagePath", storagePath);
        properties.getStringProperty("fileNameFormat", fileNameFormat);
        properties.getStringProperty("fileType", fileType);
        properties.getBoolProperty("overrideExistingFiles", overrideExistingFiles, false);

        Logger::logInfo("DataSaver override existing files: %d", overrideExistingFiles);

        if(properties.wasAnyPropertyUnknown())
        {
            std::string unknownProperties;
            properties.unknownPropertiesToString(unknownProperties);

            this->moduleFatal(absl::StrCat("Missing properties: [", unknownProperties, "]. Please sepcify the properties in the configuration file."));
            return;
        }

        // Set default media dir to getCommonDataPath().
        // Defaults to /sdcard/Android/media/app.package.name/ on Android
        // and to App Documents on iOS.
        // Same location where EventTracker stores event logs.
        absl::Status status = this->fileSaver.initialize("DataChannel", storagePath, fileNameFormat, fileType, overrideExistingFiles, this->getCommonDataPath());
        
        if(!status.ok())
        {
            this->moduleFatal(status);
        }
        Logger::logInfo("DataChannel subscribe");
        this->dataChannel = this->subscribe("DataChannel", &DataSaverModule::onData, this);
    }

    void DataSaverModule::onData(ChannelData<AnyProtoType> data)
    {
        Logger::logInfo("DataSaverModule %s on data ", this->getId().c_str());
        const AnyProtoType& value = data.getData();
        std::shared_ptr<const google::protobuf::Message> msg = value.getMessage();

        absl::Status status = this->fileSaver.onNewData(msg, data.getTimestamp());
        if(!status.ok())
        {
            moduleError(status);
        }
    }

    void DataSaverModule::terminate()
    {
        absl::Status status = this->fileSaver.endFileSaving();
        if(!status.ok())
        {
            moduleError(status);
        }
    }

}

REGISTER_MODULE(DataSaverModule, claid::DataSaverModule)