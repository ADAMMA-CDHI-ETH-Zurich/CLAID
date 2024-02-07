#pragma once

#include "dispatch/core/DataCollection/Serializer/DataSerializer.hh"
#include <fstream>

namespace claid {

class JSONSerializer : public DataSerializer
{   

    public:

        absl::Status beginNewFile(const std::string& filePath) override final;
        absl::Status finishFile() override final;
        absl::Status onNewData(std::shared_ptr<const google::protobuf::Message> data) override final;

    private:    

        std::string currentFilePath;
        std::ofstream outputFile;
       // std::shared_ptr<google::protobuf::Message> data;

};

}
