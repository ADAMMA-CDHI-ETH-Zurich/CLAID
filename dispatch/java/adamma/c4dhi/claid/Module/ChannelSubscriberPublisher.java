package adamma.c4dhi.claid.Module;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;

import javax.xml.crypto.Data;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.TypeMapping.Mutator;


public class ChannelSubscriberPublisher 
{
    private final String host;
    private Map<String, DataPackage> examplePackagesForEachChannel = new HashMap<>();
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

    // To tell the Midleware which data types our channels expect,
    // we will provide an example data package.
    private<T> DataPackage prepareExamplePackage(final String moduleId, final String channelName, Class<T> dataType, boolean isPublisher)
    {
        DataPackage.Builder builder = DataPackage.newBuilder();

        if(isPublisher)
        {
            // Only set module Id, host will be added by Middleware later.
            builder.setSourceHostModule(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        else
        {
            // Only set module Id, host will be added by Middleware later.
            builder.setTargetHostModule(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        builder.setChannel(channelName);

        DataPackage packet = builder.build();
        System.out.println("EXAMPLE PACKAGE " + packet + "\n example package end");

        T exampleInstance = TypeMapping.getNewInstance(dataType);
        if(exampleInstance == null)
        {
            System.out.println("Failed to create example instance for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        Mutator<T> mutator = TypeMapping.getMutator(exampleInstance);
        if(mutator == null)
        {
            System.out.println("Failed to create get mutator for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        packet = mutator.setPackagePayload(packet, exampleInstance);

        return packet;   
    }

    
    protected<T> Channel<T> publish(Module module, Class<T> dataType, final String channelName)
    {
        DataPackage examplePackage = prepareExamplePackage(module.getId(), channelName, dataType, true);
        if(examplePackage == null)
        {
            module.moduleError("Failed to create example package for data type \"" + dataType.getName() + "\".\n" +
                                "The data type is unsupported.");
            return null;    
        }

        this.examplePackagesForEachChannel.put(module.getId(), examplePackage);

        return new Channel<T>(module, channelName, new Publisher<T>(dataType, module.getId(), channelName, this.toModuleManagerQueue));
    }

    protected<T> Channel<T> subscribe(Module module, Class<T> dataType, final String channelName, Subscriber<T> subscriber)
    {
        DataPackage examplePackage = prepareExamplePackage(module.getId(), channelName, dataType, false);
        if(examplePackage == null)
        {
            module.moduleError("Failed to create example package for data type \"" + dataType.getName() + "\".\n" +
                                "The data type is unsupported.");

            return null;
        }

        this.examplePackagesForEachChannel.put(module.getId(), examplePackage);

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

    public ArrayList<AbstractSubscriber> getSubscribers(final String channelName, final String moduleId)
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

    public Map<String, DataPackage> getExamplePackagesForAllChannels()
    {
        return this.examplePackagesForEachChannel;
    }

}
