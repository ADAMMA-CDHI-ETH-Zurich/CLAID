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

    public ChannelSubscriberPublisher(final String host)
    {
        this.host = host;
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
            builder.setSourceHostModule(concatenateHostModuleAddress(this.host, moduleId));
        }
        else
        {
            builder.setTargetHostModule(concatenateHostModuleAddress(this.host, moduleId));
        }
        builder.setChannel(channelName);

        DataPackage packet = builder.build();

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

        return new Channel<T>(module, channelName, ChannelAccessRights.WRITE);
    }

    protected<T> Channel<T> subscribe(Module module, Class<T> dataType, final String channelName, Consumer<T> callback)
    {
        DataPackage examplePackage = prepareExamplePackage(module.getId(), channelName, dataType, false);
        if(examplePackage == null)
        {
            module.moduleError("Failed to create example package for data type \"" + dataType.getName() + "\".\n" +
                                "The data type is unsupported.");

            return null;
        }

        this.examplePackagesForEachChannel.put(module.getId(), examplePackage);

        return new Channel<T>(module, channelName, ChannelAccessRights.READ, callback);
    }

    public Map<String, DataPackage> getExamplePackagesForAllChannels()
    {
        return this.examplePackagesForEachChannel;
    }

}
