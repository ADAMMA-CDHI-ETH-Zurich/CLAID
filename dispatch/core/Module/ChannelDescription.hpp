#pragma once

#include <string>
#include <map>
#include <vector>

#include "dispatch/proto/claidservice.grpc.pb.h"

using claidservice::DataPackage;

namespace claid {

class ChannelDescription 
{
private:
    const std::string channelName;
    const std::type_info& dataType;
    std::string firstPublisherOrSubscriber;
    DataPackage templatePackage;
    std::map<std::string, std::vector<DataPackage>> modules;

    ChannelDescription(const std::string& channelName, 
            const std::type_info& dataType, 
            const std::string& firstPublisherOrSubscriber, 
            const DataPackage& templatePackage) : channelName(channelName), dataType(dataType),    
                                            firstPublisherOrSubscriber(firstPublisherOrSubscriber), templatePackage(templatePackage) 
    {

    }

public:

    template <typename T>
    static ChannelDescription newChannel(const std::string& channelName, const std::string& firstPublisherOrSubscriber) 
    {
        DataPackage templatePackage; // Create a DataPackage object.

        templatePackage.set_channel(channelName);

        Mutator<T> mutator;
        if (!TypeMapping<T>::getMutator(mutator)) 
        {
            Logger::logError("Failed to get mutator for object of class \"" + dataType.name() + "\".");
            return nullptr;
        }

        T exampleInstance;
        mutator.setPackagePayload(templatePackage, exampleInstance);

        return ChannelDescription(channelName, typeid(T), firstPublisherOrSubscriber, templatePackage);
    }

    template <typename T>
    DataPackage prepareTemplatePackage(const std::type_info& dataType, const std::string& moduleId, bool isPublisher) 
    {
        DataPackage dataPackage; // Create a DataPackage object.

        dataPackage.setChannel(channelName);

        if (isPublisher) 
        {
            // Only set module Id, host will be added by Middleware later.
            builder.set_source_host_module(moduleId); // concatenateHostModuleAddress(this.host, moduleId));
        } 
        else 
        {
            // Only set module Id, host will be added by Middleware later.
            builder.set_target_host_module(moduleId); // concatenateHostModuleAddress(this.host, moduleId));
        }

        
        Mutator<T> mutator;
        if (!TypeMapping<T>::getMutator(mutator)) 
        {
            Logger::logError("Failed to get mutator for object of class \"" + dataType.name() + "\".");
            return nullptr;
        }

        T exampleInstance;
        mutator.setPackagePayload(dataPackage, exampleInstance);

        return dataPackage;
    }

    template <typename T>
    bool dataTypeMatches(const std::type_info& dataType) {
        return this->dataType == dataType;
    }

    template <typename T>
    bool registerModuleToChannel(Module module, const std::type_info& dataType, bool isPublisher) {
        std::string moduleId = module.getId();
        if (!dataTypeMatches(dataType)) {
            std::string message = "Module \"" + moduleId + "\" failed to register for Channel \"" + channelName + "\".\n" +
                                  "Channel was created before by Module\"" + firstPublisherOrSubscriber + "\" with data type \"" + this->dataType.name() + "\", but Module\"" + moduleId +
                                  "\" tried to " + (isPublisher ? "publish" : "subscribe") + " with data type \"" + dataType.name() + "\".";

            module.moduleError(message);
            return false;
        }

        DataPackage dataPackage = prepareTemplatePackage(dataType, moduleId, isPublisher);

        if (dataPackage == nullptr) {
            module.moduleError("Failed to create example package for data type \"" + dataType.name() + "\".\n" +
                              "The data type is unsupported.");
            return false;
        }

        registerModuleToChannel(moduleId, dataPackage);

        return true;
    }

    void registerModuleToChannel(const std::string& moduleId, DataPackage packet) {
        if (this->modules.find(moduleId) == this->modules.end()) {
            this->modules[moduleId] = std::vector<DataPackage>();
        }

        this->modules[moduleId].push_back(packet);
    }

    bool doesDataTypeOfPackageMatchChannel(const DataPackage& dataPackage) {
        return dataPackage.getPayloadOneofCase() == this->templatePackage.getPayloadOneofCase();
    }

    std::vector<DataPackage> getChannelTemplatePackagesForModule(const std::string& moduleId) 
    {
        if (this->modules.find(moduleId) == this->modules.end()) 
        {
            return std::vector<DataPackage>();
        }

        return this->modules[moduleId];
    }

    DataPackage::PayloadOneofCase getPayloadOneofCase() {
        return this->templatePackage.getPayloadOneofCase();
    }
};
}


