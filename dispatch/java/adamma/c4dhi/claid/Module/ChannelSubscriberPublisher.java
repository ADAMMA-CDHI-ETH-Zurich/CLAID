package adamma.c4dhi.claid.Module;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;


import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.Mutator;


public class ChannelSubscriberPublisher 
{
    private final String host;
    private Map<String, ChannelDescription> channelDescriptions = new HashMap<>();
    
    // Map<channel, Map<module, subscriber>>
    // ArrayList, because theoretically the Module could also subscribe to the same channel twice.
    private Map<String, Map<String, ArrayList<AbstractSubscriber>>> moduleChannelsSubscriberMap = new HashMap<>();

    // Channel that is used to send DataPackages to the ModuleManager.
    // Will be provided to each published channel, allowing the post function of the Channel to send data.
    private ThreadSafeChannel<DataPackage> toModuleManagerQueue;

    public ChannelSubscriberPublisher(final String host, ThreadSafeChannel<DataPackage> toModuleManagerQueue)
    {
        this.host = host;
        this.toModuleManagerQueue = toModuleManagerQueue;
    }

    private String concatenateHostModuleAddress(final String host, final String module)
    {
        return host + ":" + module;
    }

    
    protected<T> Channel<T> publish(Module module, Class<T> dataType, final String channelName)
    {
        if(!this.channelDescriptions.containsKey(channelName))
        {
            ChannelDescription description = ChannelDescription.newChannel(channelName, dataType, module.getId());
            if(description == null)
            {
                module.moduleError("Failed to create channel \"" + channelName + "\". Unsupported data type \"" + dataType.getName() + "\".");
                return Channel.newInvalidChannel(channelName);
            }
            this.channelDescriptions.put(channelName, description);
        }

        ChannelDescription description = this.channelDescriptions.get(channelName);

        if(!description.registerModuleToChannel(module, dataType, true))
        {
            return Channel.newInvalidChannel(channelName);
        }

        return new Channel<T>(module, channelName, new Publisher<T>(dataType, module.getId(), channelName, this.toModuleManagerQueue));
    }

    protected<T> Channel<T> subscribe(Module module, Class<T> dataType, final String channelName, Subscriber<T> subscriber)
    {
        if(!this.channelDescriptions.containsKey(channelName))
        {
            this.channelDescriptions.put(channelName, ChannelDescription.newChannel(channelName, dataType, module.getId()));
        }

        ChannelDescription description = this.channelDescriptions.get(channelName);

        if(!description.registerModuleToChannel(module, dataType, false))
        {
            return Channel.newInvalidChannel(channelName);
        }

        insertSubscriber(channelName, module.getId(), (AbstractSubscriber) subscriber);

        return new Channel<T>(module, channelName, subscriber);
    }

    private void insertSubscriber(final String channelName, final String moduleId, AbstractSubscriber subscriber)
    {
        if(!moduleChannelsSubscriberMap.containsKey(channelName))
        {
            moduleChannelsSubscriberMap.put(channelName, new HashMap<>());
        }

        // Contains each Module that has at least one subscriber for the Channel called channelName.
        Map<String, ArrayList<AbstractSubscriber>> channelSubscribers = moduleChannelsSubscriberMap.get(channelName);

        if(!channelSubscribers.containsKey(moduleId))
        {
            channelSubscribers.put(moduleId, new ArrayList<>());
        }

        ArrayList<AbstractSubscriber> subscriberList = channelSubscribers.get(moduleId);
        subscriberList.add(subscriber);
    }

    public ArrayList<AbstractSubscriber> getSubscriberInstancesOfModule(final String channelName, final String moduleId)
    {
        if(!moduleChannelsSubscriberMap.containsKey(channelName))
        {
            return null;
        }

        Map<String, ArrayList<AbstractSubscriber>> channelSubscribers = moduleChannelsSubscriberMap.get(channelName);
        if(!channelSubscribers.containsKey(moduleId))
        {
            return null;
        }

        ArrayList<AbstractSubscriber> subscriberList = channelSubscribers.get(moduleId);

        return subscriberList;
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

    public boolean isDataPackageCompatibleWithChannel(DataPackage dataPackage)
    {
        final String channelName = dataPackage.getChannel();

        if(!this.channelDescriptions.containsKey(channelName))
        {
            return false;
        }

        ChannelDescription description = this.channelDescriptions.get(channelName);

        return description.doesDataTypeOfPackageMatchChannel(dataPackage);
    }

    public DataPackage.PayloadOneofCase getPayloadCaseOfChannel(final String channelName)
    {
        if(!this.channelDescriptions.containsKey(channelName))
        {
            return null;
        }

        ChannelDescription description = this.channelDescriptions.get(channelName);

        return description.getPayloadOneofCase();
    }

}
