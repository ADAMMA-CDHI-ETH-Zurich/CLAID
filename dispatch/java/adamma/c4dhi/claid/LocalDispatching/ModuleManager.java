package adamma.c4dhi.claid.LocalDispatching;
import adamma.c4dhi.claid.Module.ChannelSubscriberPublisher;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.LocalDispatching.ModuleInstanceKey;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Logger.SeverityLevel;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.InitRuntimeRequest;
import adamma.c4dhi.claid.Runtime;
import adamma.c4dhi.claid.ModuleListRequest;
import adamma.c4dhi.claid.ModuleListResponse;
import adamma.c4dhi.claid.ModuleListResponse.ModuleDescriptor;


// Takes in the ModuleDispatcher and ModuleFactories.
// Starts the ModuleDispatcher and spawns Modules accordingly.
// Forwards messages from/to the ModuleDispatcher from/to the Modules.

// Sequence to start the Runtime and register with the Middleware:
// 1. Call getModuleList() of Middleware via RPC -> send ModuleListRequest to tell the Middleware which Modules are available in this Runtime.
// 2. This call to getModuleList() will return a ModuleListResponse, which is created by the Middleware. Via this response, the Middleware tells
// us which of the Modules that we support it wants us to load based on the configuration file. This will be a subset of the Modules provided by ModuleListRequest.
// 3. Instantiate the Modules via the ModuleFactory.
// 4. Initialize the Modules, by calling their initialize functions. In the initialize function, and ONLY there, the Modules can publish/subscribe channels.
// 5. After all Modules are initialized, retrieve the list of published and subscribed channels, as well as their data types. (The data type is stored by providing an example DataPackage with the correct type).
// 6. Send this list to the Middleware by calling initRuntime() via the corresponding RPC call. 
// 7. Begin exhanging (i.e., reading and writing) packages with the middleware via the blocking sendReceivePackages() call.
public class ModuleManager 
{
    private final String hostName;
    private ModuleDispatcher dispatcher;
    private ModuleFactory moduleFactory;

    private Map<ModuleInstanceKey, Module> runningModules;

    public ModuleManager(final String hostName, ModuleDispatcher dispatcher, ModuleFactory moduleFactory)
    {
        this.hostName = hostName;
        this.dispatcher = dispatcher;
        this.moduleFactory = moduleFactory;
    }

    private boolean instantiateModule(String moduleId, String moduleClass)
    {
        if(!this.moduleFactory.isModuleClassRegistered(moduleClass))
        {
            System.out.println("ModuleManager: Failed to instantiate Module of class \"" + moduleClass + "\" (id: \")" + moduleId + "\")\n" +
            "A Module with this class is not registered to the ModuleFactory.");
            return false;
        }

        Module module = this.moduleFactory.getInstance(moduleClass, moduleId);
        this.runningModules.put(new ModuleInstanceKey(moduleId, moduleClass), module);
        return true;
    }

    private boolean instantiateModules(ModuleListResponse moduleList)
    {
        for(ModuleDescriptor descriptor : moduleList.getDescriptorsList())
        {
            String moduleId = descriptor.getModuleId();
            String moduleClass = descriptor.getModuleClass();

            if(!this.instantiateModule(moduleId, moduleClass))
            {
                System.out.println("Failed to instantiate Module \"" + moduleId + "\" (class: \"" + moduleClass + "\".\n" +
                "The Module class was not registered to the ModuleFactory.");
                return false;
            }
        }
        return true;
    }

    private boolean initializeModules(ModuleListResponse moduleList, ChannelSubscriberPublisher subscriberPublisher)
    {
        for(ModuleDescriptor descriptor : moduleList.getDescriptorsList())
        {
            String moduleId = descriptor.getModuleId();
            String moduleClass = descriptor.getModuleClass();

            ModuleInstanceKey key = new ModuleInstanceKey(moduleId, moduleClass);

            if(!this.runningModules.containsKey(key))
            {
                System.out.println("Failed to initilize Module \"" + moduleId + "\" (class: \"" + moduleClass + "\".\n" +
                "The Module class was not registered to the ModuleFactory.");
                return false;
            }

            Module module = this.runningModules.get(key);

            
            // This will call the initialize functions of each Module.
            // In the initialize function (and ONLY there), Modules can publish or subscribe Channels.
            // Hence, once all Modules have been initialized, we know the available Channels.
            module.runtimeInitialize(subscriberPublisher, descriptor.getPropertiesMap());

        }
        return true;
    }


    private boolean start()
    {
        ModuleListResponse moduleList = this.dispatcher.getModuleList(this.moduleFactory.getRegisteredModuleClasses());
        
        if(!instantiateModules(moduleList))
        {
            Logger.logFatal("ModuleDispatcher: Failed to instantiate Modules.");
            return false;
        }

        ChannelSubscriberPublisher subscriberPublisher = new ChannelSubscriberPublisher(this.hostName);

        if(!initializeModules(moduleList, subscriberPublisher))
        {
            Logger.logFatal("Failed to initialize Modules.");
            return false;
        }

        Map<String, DataPackage> examplePackagesForEachChannel = subscriberPublisher.getExamplePackagesForAllChannels();
        if(!this.dispatcher.initRuntime(examplePackagesForEachChannel))
        {
            Logger.logFatal("Failed to initialize runtime.");
            return false;
        }

        // Map<String: moduleId, List<DataPackage>: list of channels
/*        Map<String, List<DataPackage>> modules;

        if(!startRuntime())
        {
            return false;
        } */

        // init runtime
        // sendreceivepackafges
        // process queues

        return false;
    }

    

  
}