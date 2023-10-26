#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/shared_queue.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Module/AbstractSubscriber.hpp"

using claidservice::DataPackage;

namespace claid{

    class ChannelSubscriberPublisher 
    {
private:
    typedef std::pair<std::string, std::string> ChannelModulePair;

    std::string host;
    std::map<std::string, DataPackage> examplePackagesForEachChannel;
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



        Mutator<T> mutator;
        if(!TypeMapping.getMutator<T>(mutator))
        {
            Logger::logError("Failed to create get mutator for object of class \"%s\".", TypeName<T>::name());
            return null;
        }

        T exampleInstance;

        mutator.setPackagePayload(packet, exampleInstance);

        return packet;   
    }

    template<typename T>
    Channel<T> publish(Module module, const std::string& channelName)
    {
        DataPackage examplePackage = prepareExamplePackage(module.getId(), channelName, dataType, true);
        if(examplePackage == null)
        {
            module.moduleError("Failed to create example package for data type \"" + dataType.getName() + "\".\n" +
                                "The data type is unsupported.");
            return null;    
        }

        if(this.examplePackagesForEachChannel.containsKey(channelName))
        {
            if(!this.channelDataTypeNames.containsKey(channelName))
            {
                module.moduleError("Inconsistency in ChannelSubscriberPublisher. Found channel \"" + channelName + "\", but did not find it's data type.");
                return null;
            }
            
            Class<?> alreadyRegisteredDataType = this.channelDataTypeNames.get(channelName);

            if(!alreadyRegisteredDataType.equals(dataType))
            {
                module.moduleError("Subscribed twice to Channel \"" + channelName + "\", but with different data types.\n" +
                "First registration used data type \"" + alreadyRegisteredDataType.getName() + "\", second registration used \"" + dataType.getName() + "\".");
                return null;
            }
        }
        this.examplePackagesForEachChannel.put(module.getId(), examplePackage);
        this.channelDataTypeNames.put(channelName);

        return new Channel<T>(module, channelName, new Publisher<T>(dataType, module.getId(), channelName, this.toModuleManagerQueue));
    }

    template <typename T>
    Channel<T> subscribe(const std::string& moduleId, const std::string& channelName, std::shared_ptr<Subscriber<T>> subscriber) {
        DataPackage<T> examplePackage = prepareExamplePackage<T>(module.getId(), channelName, false);
        if (examplePackage == nullptr) {
            module.moduleError("Failed to create example package for data type \"" + dataType.getName() + "\".\n" +
                                "The data type is unsupported.");

            return nullptr;
        }

        this->examplePackagesForEachChannel[module.getId()] = examplePackage;

        insertSubscriber(channelName, module.getId(), (AbstractSubscriber)subscriber);

        return Channel<T>(module, channelName, subscriber);
    }

    void insertSubscriber(const std::string& channelName, const std::string& moduleId, AbstractSubscriber subscriber) {
        if (!moduleChannelsSubscriberMap.count(channelName)) {
            moduleChannelsSubscriberMap[channelName] = std::map<std::string, std::vector<AbstractSubscriber>>();
        }

        // Contains each Module that has at least one subscriber for the Channel called channelName.
        std::map<std::string, std::vector<AbstractSubscriber>>& channelSubscribers = moduleChannelsSubscriberMap[channelName];

        if (!channelSubscribers.count(moduleId)) {
            channelSubscribers[moduleId] = std::vector<AbstractSubscriber>();
        }

        std::vector<AbstractSubscriber>& subscriberList = channelSubscribers[moduleId];
        subscriberList.push_back(subscriber);
    }

    std::vector<AbstractSubscriber> getSubscribers(const std::string& channelName, const std::string& moduleId) {
        if (!moduleChannelsSubscriberMap.count(channelName)) {
            return std::vector<AbstractSubscriber>();
        }

        std::map<std::string, std::vector<AbstractSubscriber>>& channelSubscribers = moduleChannelsSubscriberMap[channelName];
        if (!channelSubscribers.count(moduleId)) {
            return std::vector<AbstractSubscriber>();
        }

        return channelSubscribers[moduleId];
    }

    std::map<std::string, DataPackage> getExamplePackagesForAllChannels() {
        return this->examplePackagesForEachChannel;
    }
};


}
