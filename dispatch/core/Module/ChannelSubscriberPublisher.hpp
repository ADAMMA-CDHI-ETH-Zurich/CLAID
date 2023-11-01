#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Module/AbstractSubscriber.hpp"

using claidservice::DataPackage;

namespace claid{

    class ChannelSubscriberPublisher 
    {
private:
    // pair<Channel, Module>
    typedef std::pair<std::string, std::string> ChannelModulePair;

    std::string host;
    std::map<ChannelModulePair, DataPackage> examplePackagesForEachModuleChannel;
    std::map<ChannelModulePair, std::vector<std::shared_ptr<AbstractSubscriber>>> moduleChannelsSubscriberMap;

    SharedQueue<DataPackage>& toModuleManagerQueue;

public:
    ChannelSubscriberPublisher(const std::string& host, SharedQueue<DataPackage>& toModuleManagerQueue)
        : host(host), toModuleManagerQueue(toModuleManagerQueue) 
    {

    }

    std::string concatenateHostModuleAddress(const std::string& host, const std::string& module) {
        return host + ":" + module;
    }

    template <typename T>
    DataPackage prepareExamplePackage(const std::string& moduleId, const std::string& channelName, bool isPublisher) 
    {
        DataPackage dataPackage;

        if(isPublisher)
        {
            // Only set module Id, host will be added by Middleware later.
            dataPackage.set_source_host_module(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        else
        {
            // Only set module Id, host will be added by Middleware later.
            dataPackage.set_target_host_module(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        dataPackage.set_channel(channelName);

        Mutator<T> mutator = TypeMapping::getMutator<T>();
        T exampleInstance;
        mutator.setPackagePayload(packet, exampleInstance);

        return packet;   
    }

    template<typename T>
    Channel<T> publish(Module& module, const std::string& channelName)
    {
        DataPackage examplePackage = prepareExamplePackage(module.getId(), channelName, true);

        const std::string& moduleId = module.getId();

        ChannelModulePair channelModuleKey = make_pair(channelName, moduleId);

        auto it = examplePackagesForEachModuleChannel.find(channelModuleKey);
        if(it != this->examplePackagesForEachModuleChannel.end())
        {
            DataPackage& alreadyRegisteredPackage = it->second;

            if(alreadyRegisteredPackage.payload_oneof_case() != examplePackage.payload_oneof_case())
            {
                const std::string alreadyRegisteredDataTypeName = dataPackagePayloadCaseToString(alreadyRegisteredPackage);
                const std::string newDataTypeName = dataPackagePayloadCaseToString(examplePackage);
                module.moduleError(absl::StrCat(
                    "Published twice Channel \"", channelName, "\", but with different data types.\n",
                    "First registration used data type \"", alreadyRegisteredDataTypeName, "\", second registration used \"", newDataTypeName, "\"."
                ));
                return null;
            }
        }
        this.examplePackagesForEachChannel.put(module.getId(), examplePackage);

        return Channel<T>(module, channelName, Publisher<T>(module.getId(), channelName, this.toModuleManagerQueue));
    }

    template <typename T>
    Channel<T> subscribe(Module& module, const std::string& channelName, std::shared_ptr<Subscriber<T>> subscriber) {
        DataPackage<T> examplePackage = prepareExamplePackage<T>(module.getId(), channelName, false);
        if (examplePackage == nullptr) {
            module.moduleError("Failed to create example package for data type \"" + dataType.getName() + "\".\n" +
                                "The data type is unsupported.");

            return nullptr;
        }
        const std::string& moduleId = module.getId();

        ChannelModulePair channelModuleKey = make_pair(channelName, moduleId);

        auto it = examplePackagesForEachModuleChannel.find(channelModuleKey);
        if(it != this->examplePackagesForEachModuleChannel.end())
        {
            DataPackage& alreadyRegisteredPackage = it->second;

            if(alreadyRegisteredPackage.payload_oneof_case() != examplePackage.payload_oneof_case())
            {
                const std::string alreadyRegisteredDataTypeName = dataPackagePayloadCaseToString(alreadyRegisteredPackage);
                const std::string newDataTypeName = dataPackagePayloadCaseToString(examplePackage);
                module.moduleError(absl::StrCat(
                    "Subscribed twice to Channel \"", channelName, "\", but with different data types.\n",
                    "First registration used data type \"", alreadyRegisteredDataTypeName, "\", second registration used \"", newDataTypeName, "\"."
                ));
                return null;
            }
        }

        this.examplePackagesForEachChannel.insert(make_pair(channelModuleKey, examplePackage));

        insertSubscriber(channelName, module.getId(), std::static_pointer_cast<AbstractSubscriber>(subscriber));

        return Channel<T>(module, channelName, subscriber);
    }

    void insertSubscriber(const std::string& channelName, const std::string& moduleId, std::shared_ptr<AbstractSubscriber> subscriber) 
    {
        ChannelModulePair channelModuleKey = make_pair(channelName, moduleId);

        auto it = moduleChannelsSubscriberMap.find(channelModuleKey);
        if(it == this->moduleChannelsSubscriberMap.end())
        {
            this->moduleChannelsSubscriberMap.insert(make_pair(channelModuleKey, std::vector<std::shared_ptr<AbstractSubscriber>>()));
        }

        this->moduleChannelsSubscriberMap[channelModuleKey].push_back(subscriber);
    }

    std::vector<std::shared_ptr<AbstractSubscriber>> getSubscriberInstancesOfModule(const std::string& channelName, const std::string& moduleId) 
    {
        ChannelModulePair channelModuleKey = make_pair(channelName, moduleId);

        auto it = moduleChannelsSubscriberMap.find(channelModuleKey);
        if(it == this->moduleChannelsSubscriberMap.end())
        {
            return std::vector<std::shared_ptr<AbstractSubscriber>>();
        }

        return it->second;
    }

    
    public ArrayList<DataPackage> getChannelTemplatePackagesForModule(final String moduleId)
    {
        ArrayList<DataPackage> templatePackages = new ArrayList<>();

        for(Map.Entry<String, ChannelDescription> channels : this.channelDescriptions.entrySet())
        {
            ArrayList<DataPackage> templatePackagesOfModuleForChannel = channels.getValue().getChannelTemplatePackagesForModule(moduleId);

            if(templatePackagesOfModuleForChannel != null)
            {
                templatePackages.addAll(templatePackagesOfModuleForChannel);
            }
        }

        return templatePackages;
    }

    bool isDataPackageCompatibleWithChannel(const DataPackage& dataPackage, const std::string& receiverModule)
    {
        const std::string& channelName = dataPackage.channel();

        ChannelModulePair channelModuleKey = make_pair(channelName, receiverModule);

        auto it = examplePackagesForEachModuleChannel.find(channelModuleKey);
        if(it != this->examplePackagesForEachModuleChannel.end())
        {
            const DataPackage& templatePackage = it->second;
            return templatePackage.payload_oneof_case() == dataPackage.payload_oneof_case();
        }
        return false;
    }

    DataPackage::PayloadOneofCase getPayloadCaseOfChannel(const std::string& channelName)
    {
        if(!this.channelDescriptions.containsKey(channelName))
        {
            return null;
        }

        ChannelDescription description = this.channelDescriptions.get(channelName);

        return description.getPayloadOneofCase();
    }
};


}
