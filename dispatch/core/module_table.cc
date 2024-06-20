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

#include "dispatch/core/module_table.hh"
#include "dispatch/core/proto_util.hh"
#include "dispatch/core/Logger/Logger.hh"
// #include <google/protobuf/text_format.h>

using grpc::Status;

using namespace std;
using namespace claid;
using namespace claidservice;

// compPacketType compares whether the 'val' packet matches the
// channel and payload in the 'ref' packet.
bool claid::compPacketType(const DataPackage& ref, const DataPackage& val) 
{
    const Blob& refBlob = ref.payload();
    const Blob& valBlob = val.payload();
    auto a = refBlob.message_type() == valBlob.message_type();
    Logger::logInfo("Dbg 1.8 %d %s %s", a, refBlob.message_type().c_str(), valBlob.message_type().c_str());

    return a;
}


// Returns true if the packet from the client is valid.
// A packet is considered valid when
//    - the channel and payload are set
//    - the payload is not a control package
//    - the source or the target are set (exclusive or)
//
bool claid::validPacketType(const DataPackage& ref) {
    return !(ref.channel().empty() ||
        (ref.source_module().empty() && ref.target_module().empty()) ||
        (!ref.source_module().empty() && !ref.target_module().empty()) ||
        ref.has_control_val());
}

SharedQueue<DataPackage>* ModuleTable::lookupOutputQueue(const string& moduleId) {
    for(auto& entry : moduleRuntimeMap)
    {
        Logger::logInfo("%s %d", entry.first.c_str(), entry.second);
    }
    auto rt = moduleRuntimeMap[moduleId];
    if (rt != Runtime::RUNTIME_UNSPECIFIED) {
        auto outQueue = runtimeQueueMap[rt];
            Logger::logWarning("Output queue size: %d", outQueue->size());

        if (outQueue) {
            return outQueue.get();
        }
    }
    else
    {
        Logger::logError("ModuleTable::lookupOutputQueue runtime is unspecified");
    }
    return nullptr;
}

std::vector<std::shared_ptr<SharedQueue<claidservice::DataPackage>>> ModuleTable::getRuntimeQueues()
{
    std::vector<std::shared_ptr<SharedQueue<claidservice::DataPackage>>> queues;
    for(auto& entry : runtimeQueueMap)
    {
        queues.push_back(entry.second);
    }
    return queues;
}


void ModuleTable::setProperties(const ModuleTableProperties& props) {
    this->props = props;
}

void ModuleTable::setNeededModule(const string& moduleId, const string& moduleClass,
        const google::protobuf::Struct& properties) {
    // TODO: verify no module is registered incorreclty multiple times
    moduleToClassMap[moduleId] = moduleClass;
    moduleProperties[moduleId] = properties;
}

void ModuleTable::setModuleChannelToConnectionMappings(const std::string& moduleId,
      const std::map<std::string, std::string>& inputChannelToConnectionMappping,
      const std::map<std::string, std::string>& outputChannelToConnectionMapping)
{
    moduleInputChannelsToConnectionMap[moduleId] = inputChannelToConnectionMappping;
    moduleOutputChannelsToConnectionMap[moduleId] = outputChannelToConnectionMapping;
}

void ModuleTable::setExpectedChannel(const string& channelId, const string& source, const string& target) {
    unique_lock<shared_mutex> lock(chanMapMutex);

    // TODO: Check whether the input doesn't conflict with
    // assumptions and previous additons of channels.
    chanMap[channelId].sources[source] = false;
    chanMap[channelId].targets[target] = false;
}

Status ModuleTable::setChannelTypes(const string& moduleId,
            const google::protobuf::RepeatedPtrField<DataPackage>& channels) {
    if (moduleId.empty()) {
        return Status(grpc::INVALID_ARGUMENT, "Module id must not be empty");
    }

    // Broadly lock until the entire input is processed. This makes this
    // code simpler and since this is only during runtime setup it's not the most
    // performance critical section.
    unique_lock<shared_mutex> lock(chanMapMutex);
    for(auto& chanPkt : channels) {
        // Validate the input packets, which guarantees that all required fields are set.
        if (!validPacketType(chanPkt)) {
            return Status(grpc::INVALID_ARGUMENT,
                "Invalid packet type for channel '" +
                chanPkt.channel() + "' : " + messageToString(chanPkt));
        }

        string channelId = chanPkt.channel();

        const string& src = chanPkt.source_module();
        const string& tgt = chanPkt.target_module();

        // At least source or target has to be the module in question, i.e., moduleId is either
        // subscriber or publisher for this channel.
        if ((moduleId != src) && (moduleId != tgt)) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
                "Invalid source/target Module for Channel \"", channelId, "\".\n",
                "Module \"", moduleId, "\" has to be either publisher or subscriber of the Channel, but neither was set."));
        }

        // Module is publisher. The Module is an output channel of the Module.
        if(src == moduleId)
        {
            if(moduleOutputChannelsToConnectionMap[moduleId][channelId] == "")
            {
                Logger::logWarning("%s", absl::StrCat(
                "Invalid published channel \"", channelId, "for Module \"", moduleId, "\".", 
                "Could not find where this Channel is connected/mapped to, as it was not found in the moduleOutputChannelsToConnectionMap.\n",
                "Was the channel specified in the \"output_channels\" property of the Module?").c_str());
                continue;
            }

            // Map the channel name to the corresponding connection.
            channelId = moduleOutputChannelsToConnectionMap[moduleId][channelId];
        }
        // Module is subscriber. The channel is an input channel of the Module.
        else if(tgt == moduleId)
        {
            if(moduleInputChannelsToConnectionMap[moduleId][channelId] == "")
            {
                Logger::logWarning("%s", absl::StrCat(
                "Invalid subscribed channel \"", channelId, " for Module \"", moduleId, "\".", 
                "Could not find where this Channel is connected/mapped to, as it was not found in the moduleInputChannelsToConnectionMap.\n",
                "Was the channel specified in the \"input_channels\" property of the Module?").c_str());
                continue;
            }

            // Map the channel name to the corresponding connection.
            channelId = moduleInputChannelsToConnectionMap[moduleId][channelId];
        }

        // Find the channel and verify and set the type.
        ChannelEntry* entry = findChannel(channelId);
        if (!entry) {
            Logger::logWarning("%s", absl::StrCat("Channel \"", channelId, "\" not known, did you specify it in the configuration file?").c_str());
            continue;
        }

        // If the type doesn't match we return an error.
        if (entry->isPayloadTypeSet() && entry->getPayloadType() != chanPkt.payload().message_type()) 
        {
            // If subscribed channel is of type claid.CLAIDANY, we make an exception.s
            if(entry->getPayloadType() != "claidservice.CLAIDANY")
            {
                return Status(grpc::INVALID_ARGUMENT, absl::StrCat("Invalid packet type for channel '",chanPkt.channel(), "' : ",
                "Payload type is ", entry->getPayloadType(), " but expected ", chanPkt.payload().message_type()));
            }
        }

        // Mark the source / target as matched by this module.
        bool isSource = (moduleId == src);
        const string& val = isSource ? src : tgt;
        if (!entry->addSet(val, isSource)) {
            return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
                "Module \"", val, "\" ", isSource ? "published" : "subscribed", " channel \"", channelId, "\", ",
                "but the channel was not specified in the configuration file. Make sure to add the channel as ", 
                isSource ? "output channel" : "input channel", " of Module \"", val,  "\" in the configuration file."
            ));
        }
        Logger::logInfo("Setting channel %s %s", channelId.c_str(), chanPkt.payload().message_type().c_str());

        // if(chanPkt.payload_oneof_case() == DataPackage::PayloadOneofCase::PAYLOAD_ONEOF_NOT_SET)
        // {
        //     return Status(grpc::INVALID_ARGUMENT, absl::StrCat(
        //         "Failed to set channel \"", channelId, "\". Payload type is not set."
        //     ));
        // }

        entry->setPayloadType(chanPkt.payload().message_type());
    }
    return Status::OK;
}

bool ModuleTable::ready() const {
    for(auto& it : chanMap) {
        for(auto& innerIt : it.second.sources) {
            if (!innerIt.second) {
                return false;
            }
        }

        for(auto& innerIt : it.second.targets) {
            if (!innerIt.second) {
                return false;
            }
        }
        if (!it.second.isPayloadTypeSet()) {
            return false;
        }
    }
    return true;
}

const ChannelEntry* ModuleTable::isValidChannel(const DataPackage& pkt, bool ignorePayload /*= false*/) const {

    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(chanMapMutex));

    auto entry = (const_cast<ModuleTable*>(this))->findChannel(pkt.channel());
    if (!entry) {
        Logger::logError("Could not find channel \"%s\" in ModuleTable", pkt.channel().c_str());
        return nullptr;
    }

    auto srcIt = entry->sources.find(pkt.source_module());
    if (srcIt == entry->sources.end()) {
        Logger::logError("Unknown source for channel \"%s\"", pkt.channel().c_str());
        return nullptr;
    }

    if(ignorePayload)
    {
        return entry;
    }
    if (entry->getPayloadType() != pkt.payload().message_type()) {
        Logger::logError("Invalid package, payload type mismatch! Expected \"%s\" but got \"%s\"", entry->getPayloadType().c_str(), pkt.payload().message_type().c_str());
        return nullptr;
    }


    return entry;
}

void ModuleTable::forwardPackageToAllSubscribers(const claidservice::DataPackage& pkt,
                          const ChannelEntry* chanEntry,
                          SharedQueue<claidservice::DataPackage>& queue) const {

    Logger::logInfo("ModuleTable add output packets");
    // Make a copy of the incoming packet and augment it.
    auto cpPkt = pkt;
    augmentFieldValues(cpPkt);
    for(auto& it : chanEntry->targets) 
    {
        const std::string& targetModuleName = it.first;
        Logger::logInfo("ModuleTable add %s ", targetModuleName.c_str());

        auto moduleConnectionIt = this->moduleInputChannelsToConnectionMap.find(targetModuleName);

        if(moduleConnectionIt == this->moduleInputChannelsToConnectionMap.end())
        {
            Logger::logWarning("ModuleTable could not find Module \"%s\" in moduleInputChannelsToConnectionMap.", targetModuleName.c_str());
            continue;
        }
        const std::map<std::string, std::string>& targetModuleInputChannelMappings = moduleConnectionIt->second;
        for(const auto& entry : targetModuleInputChannelMappings)
        {
            Logger::logInfo("ModuleTable add output packets %s %s", entry.first.c_str(), entry.second.c_str());
            // Look for all Channels of the Module that map to the connection of the Channel.
            // Map the connection name to the individual channel name of the Module.
            // This allows Modules to arbitrarily name their input and output channels, the actual connection
            // is defined by the user in the configuration file.
            if(entry.second == pkt.channel())
            {
                auto outPkt = make_shared<DataPackage>(cpPkt);
                outPkt->set_target_module(targetModuleName);

                outPkt->set_channel(entry.first); // Set channel name to the channel name of the module (which maps to the connection).
                Logger::logInfo("ModuleTable debug Received package from %s %s %s", pkt.source_module().c_str(), entry.first.c_str(), entry.second.c_str());
                queue.push_back(outPkt);
            }            
        }
    }
}

// The DataPackage pkt was posted to a Channel by a Module.
// We forward the pkt to all non-module entities which have subscribed to all data posted to the particular
// Module on that channel. This is different from regular channel subscriptions.
// In regular subscriptions, data is forwarded to all subscribers to a channel, no matter what Module posted the data.
// In loose direct subscriptions, data is forwarded directly to separate non-module subscribers to a channel, but only if the data 
// was posted by the Module that the non-module subscriber explicitly subscribed to.
// This typically is used for UI visualizations, allowing UI widgets to subscribe to data posted by a certain Module.
// For example, check out CLAIDModuleView of FlutterCLAID.
void ModuleTable::forwardPackageOfModuleToAllLooseDirectSubscribers(
        const claidservice::DataPackage& pkt, SharedQueue<claidservice::DataPackage>& queue) const
{
    //     std::map<std::string, std::map<Runtime, std::vector<claidservice::LooseChannelSubscription>>> looseDirectChannelSubscriptions;
    const std::string& channelName = pkt.channel();
    auto it = this->looseDirectChannelSubscriptions.find(channelName);

    if(it != this->looseDirectChannelSubscriptions.end())
    {
        for(const auto& runtimeSubscribers : it->second)
        {
            for(const claidservice::LooseDirectChannelSubscription& subscription : runtimeSubscribers.second)
            {
                if(pkt.source_module() == subscription.subscribed_module())
                {
                    std::shared_ptr<DataPackage> newPackage = std::make_shared<DataPackage>(pkt);
                    ControlPackage* controlVal = newPackage->mutable_control_val();
                    controlVal->set_ctrl_type(CtrlType::CTRL_DIRECT_SUBSCRIPTION_DATA);
                    (*controlVal->mutable_loose_direct_subscription()) = subscription;
                    controlVal->set_runtime(subscription.subscriber_runtime());
                    queue.push_back(newPackage);
                }
            }
        }
    }

}

void ModuleTable::augmentFieldValues(claidservice::DataPackage& pkt) const {
    pkt.set_source_user_token(props.userId);
    pkt.set_device_id(props.deviceId);

    // TODO: Add more meta data fields here, like time stamps etc.
}

ChannelEntry* ModuleTable::findChannel(const string& channelId) {
    auto it = chanMap.find(channelId);
    if (it == chanMap.end()) {
        return nullptr;
    }
    return &it->second;
}

const string ModuleTable::toString() const {
    std::ostringstream out;
    out << "Modules: " << moduleToClassMap.size() << endl;
    for(auto& it : moduleToClassMap) {
        out << "    " << it.first << " ==> " << it.second << endl;
        out << "        Properties: " << endl;
        auto props = moduleProperties.find(it.first);
        if (props != moduleProperties.end()) {
            
            out << "            " <<  messageToString(props->second) << endl;
            
        }
    }
    out << "Channels:" << endl;

    shared_lock<shared_mutex> lock(const_cast<shared_mutex&>(chanMapMutex));
    for(auto& it : chanMap) {
        out << "    \'" << it.first << "\'" << endl;
        out << "         Sources: ";
        for(auto& srcIt : it.second.sources) {
            out << srcIt.first << " : " << (srcIt.second ? "true" : "false") << "   ";
        }
        out << endl;

        out << "         Targets: ";
        for(auto& tgtIt : it.second.targets) {
            out << tgtIt.first << " : " << (tgtIt.second ? "true" : "false") << "  ";
        }
        out << endl;

        out << "         Type:" << it.second.getPayloadType() << endl;
    }
    return out.str();
}

void ModuleTable::addRuntimeIfNotExists(const claidservice::Runtime& runtime)
{
    auto it = runtimeQueueMap.find(runtime);
    if(it == runtimeQueueMap.end())
    {
        runtimeQueueMap.insert(make_pair(runtime, make_shared<SharedQueue<DataPackage>>()));
    }
}

std::shared_ptr<SharedQueue<claidservice::DataPackage>> ModuleTable::getOutputQueueOfRuntime(const claidservice::Runtime& runtime)
{
    auto it = runtimeQueueMap.find(runtime);
    if(it == runtimeQueueMap.end())
    {
        return nullptr;
    }
    return it->second;
}


// ONLY FOR TESTING REMOVE LATER
void ModuleTable::addModuleToRuntime(const std::string& moduleID, claidservice::Runtime runtime)
{
    moduleRuntimeMap[moduleID] = runtime;
    auto outQueue = runtimeQueueMap[runtime];
    if(!outQueue)
    {
        runtimeQueueMap[runtime] = make_shared<SharedQueue<DataPackage>>();
    }
}

bool ChannelEntry::addSet(const string& key, bool isSrc) {
    map<string, bool>& field = (isSrc) ? sources : targets;
    auto it = field.find(key);
    if (it == field.end()) {
        field[key] = false;
        return false;
    }
    it->second = true;
    return true;
}


void ModuleTable::appendModuleAnnotations(const std::map<std::string, claidservice::ModuleAnnotation>& moduleAnnotations)
{
    this->moduleAnnotations.insert(moduleAnnotations.begin(), moduleAnnotations.end());
}

const std::map<std::string, claidservice::ModuleAnnotation> ModuleTable::getModuleAnnotations() const
{
    return this->moduleAnnotations;
}

bool ModuleTable::getAnnotationForModule(const std::string& moduleClass, claidservice::ModuleAnnotation& annotation) const
{
    auto it = this->moduleAnnotations.find(moduleClass);
    if(it == this->moduleAnnotations.end())
    {
        return false;
    }

    annotation = it->second;
    return true;
}

void ModuleTable::getRunningModules(std::vector<std::string>& moduleIDs) const
{
    moduleIDs.clear();

    for(const auto& entry : this->moduleClassRuntimeMap)
    {
        moduleIDs.push_back(entry.first);
    }
}

size_t ModuleTable::getNumberOfRunningRuntimes()
{
    return this->runtimeQueueMap.size();
}

void ModuleTable::clearLookupTables()
{
    unique_lock<shared_mutex> lock(chanMapMutex);
    moduleClassRuntimeMap.clear();
    // Not necessary
    // RuntimeQueueMap.clear();
    moduleToClassMap.clear();
    moduleProperties.clear();
    moduleRuntimeMap.clear();
    chanMap.clear(),
    moduleAnnotations.clear();
    moduleInputChannelsToConnectionMap.clear();
    moduleOutputChannelsToConnectionMap.clear();


}

bool ModuleTable::lookupOutputConnectionForChannelOfModule(const std::string& sourceModule, const std::string& channelName, std::string& connectionName) const
{
    auto it = this->moduleOutputChannelsToConnectionMap.find(sourceModule);
    if(it == this->moduleOutputChannelsToConnectionMap.end())
    {
        return false;
    }

    const std::map<std::string, std::string>& outputChannelMappingsOfModule = it->second;

    auto it2 = outputChannelMappingsOfModule.find(channelName);
    if(it2 == outputChannelMappingsOfModule.end())
    {
        return false;
    }

    connectionName = it2->second;
    return true;
}

bool ModuleTable::getTypeOfModuleWithId(const std::string& moduleId, std::string& moduleType)
{
    auto it = moduleToClassMap.find(moduleId);
    if(it == moduleToClassMap.end())
    {
        return false;
    }

    moduleType = it->second;
    return true;
}


const std::map<std::string, std::string>& ModuleTable::getModuleToClassMap()
{
    return this->moduleToClassMap;
}


void ModuleTable::addLooseDirectSubscription(claidservice::LooseDirectChannelSubscription& subscription)
{
    unique_lock<shared_mutex> lock(chanMapMutex);
    const std::string& channel = subscription.subscribed_channel();
    const claidservice::Runtime& runtime = subscription.subscriber_runtime();

    this->looseDirectChannelSubscriptions[channel][runtime].push_back(subscription);
}

void ModuleTable::removeLooseDirectSubscription(claidservice::LooseDirectChannelSubscription& subscription)
{
    unique_lock<shared_mutex> lock(chanMapMutex);
    const std::string& channel = subscription.subscribed_channel();
    const claidservice::Runtime& runtime = subscription.subscriber_runtime();

    std::vector<claidservice::LooseDirectChannelSubscription>& subscriptions = this->looseDirectChannelSubscriptions[channel][runtime];

    std::vector<claidservice::LooseDirectChannelSubscription> subscriptionsToKeep;

    for(const claidservice::LooseDirectChannelSubscription& existingSubscription : subscriptions)
    {
        // Runtime subscriber_runtime = 1; 
        // string subscriber_entity = 2;
        // string subscribed_module = 3;
        // string subscribed_channel = 4; 
        // Check if the subscription in the vector is not equal to the subscription to remove.
        // If it is not equal, keep it by pushing it into the "subscriptionsToKeep" vector.
        if(!(
            existingSubscription.subscriber_runtime() == subscription.subscriber_runtime() &&
            existingSubscription.subscriber_entity() == subscription.subscriber_entity() &&
            existingSubscription.subscribed_module() == subscription.subscribed_module() && 
            existingSubscription.subscribed_channel() == subscription.subscribed_channel()
        ))
        {
            subscriptionsToKeep.push_back(existingSubscription);
        }
    }

    subscriptions = subscriptionsToKeep;
}

void ModuleTable::removeAllLooseDirectSubscriptionsOfRuntime(claidservice::Runtime runtime)
{
    unique_lock<shared_mutex> lock(chanMapMutex);
    // For each channel, check if the runtime has any direct subscribers.
    // If so, erase the entry from the map.
    for(auto& entry : this->looseDirectChannelSubscriptions)
    {
        std::map<Runtime, 
            std::vector<LooseDirectChannelSubscription>>& channelRuntimeSubscriptions = entry.second;
        
        auto itRuntime = channelRuntimeSubscriptions.find(runtime);
        if(itRuntime != channelRuntimeSubscriptions.end())
        {
            channelRuntimeSubscriptions.erase(itRuntime);
        }

    }
}

bool ModuleTable::isModulePublishingChannel(const std::string& moduleId, const std::string& channel)
{
    DataPackage package;
    package.set_source_module(moduleId);
    package.set_channel(channel);

    return isValidChannel(package, true) != nullptr;
}
