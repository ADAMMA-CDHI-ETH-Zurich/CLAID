package adamma.c4dhi.claid.Module;

import java.util.ArrayList;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;

import java.util.Map;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class ChannelDescription 
{
    final String channelName;
    final DataType dataType;

    // Module in the Java runtime that was the first to subscribe to or publish this channel.
    final String firstPublisherOrSubscriber; 

    // Package with channel name and payloadcase (data type) defined.
    // Is used to compare the payloadcase of incoming packages with the payloadcase of this channel,
    // to find out if the data type is correct.
    // Also, this package is used as template when registering Modules to create template packages for each registerd Module
    // i.e.: source | target | payload
    final DataPackage templatePackage;

    // This Map holds all Modules that are publishing or subscribing this Channel.
    // <ModuleName, List of packages defining the channel with source | target | payload for each Module>
    // Map<moduleId, packages>
    Map<String, ArrayList<DataPackage>> modules = new HashMap<>();

    private ChannelDescription(final String channelName, 
        final DataType dataType, final String firstPublisherOrSubscriber, final DataPackage templatePackage)
    {
        this.channelName = channelName;
        this.dataType = dataType;
        this.firstPublisherOrSubscriber = firstPublisherOrSubscriber;
        this.templatePackage = templatePackage;
    }

    public static<T> ChannelDescription newChannel(final String channelName, final DataType dataType, final String firstPublisherOrSubscriber)
    {
        DataPackage.Builder builder = DataPackage.newBuilder();
        builder.setChannel(channelName);

        DataPackage packet = builder.build();
        System.out.println("EXAMPLE PACKAGE " + packet + "\n example package end");

        T exampleInstance = TypeMapping.getNewInstance(dataType);
        if(exampleInstance == null)
        {
            System.out.println("Failed to create example instance for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        Mutator<T> mutator = TypeMapping.getMutator(dataType);
        if(mutator == null)
        {
            System.out.println("Failed to create get mutator for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        packet = mutator.setPackagePayload(packet, exampleInstance);

        return new ChannelDescription(channelName, dataType, firstPublisherOrSubscriber, packet);

    }

    public<T> DataPackage prepareTemplatePackage(DataType dataType, final String moduleId, boolean isPublisher)
    {
        DataPackage.Builder builder = DataPackage.newBuilder();
        builder.setChannel(channelName);

        if(isPublisher)
        {
            // Only set module Id, host will be added by Middleware later.
            builder.setSourceModule(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        else
        {
            // Only set module Id, host will be added by Middleware later.
            builder.setTargetModule(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        DataPackage dataPackage = builder.build();

        T exampleInstance = TypeMapping.getNewInstance(dataType);
        if(exampleInstance == null)
        {
            System.out.println("Failed to create example instance for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        Mutator<T> mutator = TypeMapping.getMutator(dataType);
        if(mutator == null)
        {
            System.out.println("Failed to create get mutator for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        dataPackage = mutator.setPackagePayload(dataPackage, exampleInstance);

        return dataPackage;
    }

    private<T> boolean dataTypeMatches(DataType dataType)
    {
        return this.dataType.equals(dataType);
    }

    public<T> boolean registerModuleToChannel(Module module, DataType dataType, boolean isPublisher)
    {
        final String moduleId = module.getId();
        if(!dataTypeMatches(dataType))
        {
            String message = "Failed to register for Channel \"" + channelName + "\".\n" +
            "Channel was created before by Module\" " + firstPublisherOrSubscriber + "\" with data type \"" + this.dataType.getName() + "\", but Module\"" + moduleId + 
            "\" tried to " + (isPublisher ? "publish" : "subscribe") + " with data type \"" + dataType.getName() + "\".";

            module.moduleError(message);
            return false;
        }
        

        DataPackage dataPackage = prepareTemplatePackage(dataType, moduleId, isPublisher);

        if(dataPackage == null)
        {
            module.moduleError("Failed to create example package for data type \"" + dataType.getName() + "\".\n" +
                                "The data type is unsupported.");
            return false;    
        }

        registerModuleToChannel(moduleId, dataPackage);

        return true;   
    }

    private void registerModuleToChannel(final String moduleId, DataPackage packet)
    {
        if(!this.modules.containsKey(moduleId))
        {
            this.modules.put(moduleId, new ArrayList<DataPackage>());
        }

        this.modules.get(moduleId).add(packet);
    }
  

    public boolean doesDataTypeOfPackageMatchChannel(DataPackage dataPackage)
    {
        return dataPackage.getPayloadOneofCase() == this.templatePackage.getPayloadOneofCase(); 
    }

    public ArrayList<DataPackage> getChannelTemplatePackagesForModule(final String moduleId)
    {
        if(!this.modules.containsKey(moduleId))
        {
            return null;
        }

        return this.modules.get(moduleId);
    }

    public DataPackage.PayloadOneofCase getPayloadOneofCase()
    {
        return this.templatePackage.getPayloadOneofCase();
    }
}
