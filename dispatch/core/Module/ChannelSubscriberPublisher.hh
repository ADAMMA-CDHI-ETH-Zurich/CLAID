#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Module/AbstractSubscriber.hh"
#include "dispatch/core/Module/Module.hh"
#include "dispatch/core/Module/Channel.hh"

using claidservice::DataPackage;

namespace claid
{

    class ChannelSubscriberPublisher 
    {
private:



    // Map<ModuleId, list of example packages defining publishments and subscriptions of the Module for that Channel>.
    std::map<std::string, std::vector<DataPackage>> examplePackagesForEachModule;

    // pair<ChannelId, ModuleId>
    typedef std::pair<std::string, std::string> ChannelModulePair;
    std::map<ChannelModulePair, std::vector<std::shared_ptr<AbstractSubscriber>>> moduleChannelsSubscriberMap;

    SharedQueue<DataPackage>& toModuleManagerQueue;

public:
    ChannelSubscriberPublisher(SharedQueue<DataPackage>& toModuleManagerQueue)
        : toModuleManagerQueue(toModuleManagerQueue) 
    {

    }


    template <typename T>
    DataPackage prepareExamplePackage(const std::string& moduleId, const std::string& channelName, bool isPublisher) 
    {
        DataPackage dataPackage;

        if(isPublisher)
        {
            // Only set module Id, host will be added by Middleware later.
            dataPackage.set_source_module(moduleId);//concatenateHostModuleAddress(this->host, moduleId));
        }
        else
        {
            // Only set module Id, host will be added by Middleware later.
            dataPackage.set_target_module(moduleId);//concatenateHostModuleAddress(this->host, moduleId));
        }
        dataPackage.set_channel(channelName);

        Mutator<T> mutator = TypeMapping::getMutator<T>();
        T exampleInstance;
        mutator.setPackagePayload(dataPackage, exampleInstance);

        return dataPackage;   
    }

    template<typename T>
    Channel<T> publish(ModuleRef module, const std::string& channelName)
    {
        const std::string moduleId = module.getId();
        DataPackage examplePackage = prepareExamplePackage<T>(moduleId, channelName, true);



        this->examplePackagesForEachModule[moduleId].push_back(examplePackage);

        return Channel<T>(module, channelName, Publisher<T>(moduleId, channelName, this->toModuleManagerQueue));
    }

    template <typename T>
    Channel<T> subscribe(ModuleRef module, const std::string& channelName, std::shared_ptr<Subscriber<T>> subscriber) 
    {
        std::string moduleId = module.getId();
        DataPackage examplePackage = prepareExamplePackage<T>(moduleId, channelName, false);


        this->examplePackagesForEachModule[moduleId].push_back(examplePackage);

        insertSubscriber(channelName, moduleId, std::static_pointer_cast<AbstractSubscriber>(subscriber));

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

    
    std::vector<DataPackage> getChannelTemplatePackagesForModule(const std::string& moduleId)
    {
        // If moduleId does not exist in the map, it is added automatically and a new empty vector is allocated.
       return examplePackagesForEachModule[moduleId];
    }

    bool isDataPackageCompatibleWithChannel(const DataPackage& dataPackage, const std::string& receiverModule)
    {
        const std::string& channelName = dataPackage.channel();

        
        auto it = examplePackagesForEachModule.find(receiverModule);
        if(it != this->examplePackagesForEachModule.end())
        {

            for(const DataPackage& templatePackage : it->second)
            {
                // Look for the first etmplate package of the Module for this channel.
                // If the Module has subscribed/published the same Channel multiple times,
                // then all the payload cases should be identical (checked by the Middleware during InitRuntime).
                // What if this function is used before InitRuntime? Then it is not guaranteed that the payload cases are verified.
                if(templatePackage.channel() == channelName)
                {
                    return templatePackage.payload_oneof_case() == dataPackage.payload_oneof_case();
                }

            }

        }
        return false;
    }

    DataPackage::PayloadOneofCase getPayloadCaseOfChannel(const std::string& channelName, const std::string& receiverModule)
    {    
        auto it = examplePackagesForEachModule.find(receiverModule);
        if(it != this->examplePackagesForEachModule.end())
        {
            for(const DataPackage& templatePackage : it->second)
            {
                // Look for the first etmplate package of the Module for this channel.
                // If the Module has subscribed/published the same Channel multiple times,
                // then all the payload cases should be identical (checked by the Middleware during InitRuntime).
                // What if this function is used before InitRuntime? Then it is not guaranteed that the payload cases are verified.
                if(templatePackage.channel() == channelName)
                {
                    return templatePackage.payload_oneof_case();
                }
            }
        }
        return DataPackage::PayloadOneofCase::PAYLOAD_ONEOF_NOT_SET;
    }
};


}
