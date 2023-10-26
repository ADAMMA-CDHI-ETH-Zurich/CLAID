package adamma.c4dhi.claid.LocalDispatching;
import adamma.c4dhi.claid.Module.AbstractSubscriber;
import adamma.c4dhi.claid.Module.ChannelSubscriberPublisher;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.Module.ThreadSafeChannel;
import adamma.c4dhi.claid.LocalDispatching.ModuleInstanceKey;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Logger.SeverityLevel;

import java.util.Map;
import java.util.function.Consumer;

import javax.xml.crypto.Data;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

import java.lang.reflect.InvocationTargetException;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.InitRuntimeRequest;
import adamma.c4dhi.claid.Runtime;
import adamma.c4dhi.claid.ModuleListRequest;
import adamma.c4dhi.claid.ModuleListResponse;
import adamma.c4dhi.claid.ModuleListResponse.ModuleDescriptor;


import io.grpc.stub.StreamObserver;


// Takes in the ModuleDispatcher and ModuleFactories.
// Starts the ModuleDispatcher and spawns Modules accordingly.
// Forwards messages from/to the ModuleDispatcher from/to the Modules.

// Sequence to start the Runtime and register with the Middleware:
// 1. Call getModuleList() of Middleware via RPC -> send ModuleListRequest to tell the Middleware which Modules are available in this Runtime.
// 2. This call to getModuleList() will return a ModuleListResponse, which is created by the Middleware. Via this response, the Middleware tells
// us which of the Modules that it wants us to load based on the configuration file.
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

    // ModuleId, Module
    private Map<String, Module> runningModules = new HashMap<>();

    ChannelSubscriberPublisher subscriberPublisher;

    StreamObserver<DataPackage> inStream;
    StreamObserver<DataPackage> outStream;

    ThreadSafeChannel<DataPackage> fromModulesChannel = new ThreadSafeChannel<>();
    Thread readFromModulesThread;
    boolean running = false;

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
            System.out.println("ModuleManager: Failed to instantiate Module of class \"" + moduleClass + "\" (id: " + moduleId + "\")\n" +
            "A Module with this class is not registered to the ModuleFactory.");
         
            // The Middleware always returns all Modules named in the config file.
            // If we cannot load it, we assume another registered runtime can.
            // Therefore, no error.
            return true;
        }

        Logger.logInfo("Loaded Module id \"" + moduleId + "\" (class: \"" + moduleClass + "\").");
        Module module = this.moduleFactory.getInstance(moduleClass, moduleId);
        this.runningModules.put(moduleId, module);
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

            String key = moduleId;
            System.out.println(key + "  " + this.runningModules);
            if(!this.runningModules.containsKey(key))
            {
                System.out.println("Failed to initialize Module \"" + moduleId + "\" (class: \"" + moduleClass + "\".\n" +
                "The Module was not loaded.");
                
                // The middleware always sends all Modules parsed from the configuration file.
                // If this Module is not running in this runtime, that means we could not instantiate it before.
                // Hence, we assume this Module is supported by another runtime.
                continue;
            }

            Module module = this.runningModules.get(key);

            
            // This will call the initialize functions of each Module.
            // In the initialize function (and ONLY there), Modules can publish or subscribe Channels.
            // Hence, once all Modules have been initialized, we know the available Channels.
            Logger.logInfo("Calling module.start() for Module \"" + module.getId() + "\".");
            module.start(subscriberPublisher, descriptor.getPropertiesMap());
            Logger.logInfo("Module \"" + module.getId() + "\" has started.");

        }
        return true;
    }

    // A Channel is defined by a template package defining source | target | payload.
    // For each Module, we have a list of DataPackages defining all Channels published or subscribed by the Module.
    private Map<String, ArrayList<DataPackage>> getTemplatePackagesOfModules()
    {
        Map<String, ArrayList<DataPackage>> moduleChannels = new HashMap<>();
        for(String moduleId : this.runningModules.keySet())
        {
            ArrayList<DataPackage> templatePackagesForModule = this.subscriberPublisher.getChannelTemplatePackagesForModule(moduleId);
            moduleChannels.put(moduleId, templatePackagesForModule);
        }
        return moduleChannels;
    }


    public boolean start()
    {
        System.out.println(this.moduleFactory.getRegisteredModuleClasses());
        ModuleListResponse moduleList = this.dispatcher.getModuleList(this.moduleFactory.getRegisteredModuleClasses());
        
        if(!instantiateModules(moduleList))
        {
            Logger.logFatal("ModuleDispatcher: Failed to instantiate Modules.");
            return false;
        }

        this.subscriberPublisher = new ChannelSubscriberPublisher(this.hostName, this.fromModulesChannel);

        if(!initializeModules(moduleList, subscriberPublisher))
        {
            Logger.logFatal("Failed to initialize Modules.");
            return false;
        }

        Map<String, ArrayList<DataPackage>> examplePackagesOfModules = getTemplatePackagesOfModules();
        if(!this.dispatcher.initRuntime(examplePackagesOfModules))
        {
            Logger.logFatal("Failed to initialize runtime.");
            return false;
        }


                
        if(!this.dispatcher.sendReceivePackages(receivedPackage -> onDataPackageReceived(receivedPackage)))
        {
            Logger.logFatal("Failed to set up input and output streams with middleware.");
            return false;
        }
        
        

        this.running = true;
        this.readFromModulesThread = new Thread(){
            public void run(){
              readFromModules();
            }
          };
        this.readFromModulesThread.start();

        // Map<String: moduleId, List<DataPackage>: list of channels
/*        Map<String, List<DataPackage>> modules;

        if(!startRuntime())
        {
            return false;
        } */

        // init runtime
        // sendreceivepackafges
        // process queues

        return true;
    }

    private String[] splitHostModule(String addr)
    {
        String[] hostAndModule = addr.split(":");

        if(hostAndModule.length != 2)
        {
            return null;
        }

        return hostAndModule;
    }

    private void onDataPackageReceived(DataPackage dataPackage)
    {
        /*String[] hostAndModule = splitHostModule(dataPackage.getTargetHostModule());

        if(hostAndModule == null)
        {
            Logger.logError("Java Runtime received DataPackage with invalid address \"" + dataPackage.getTargetHostModule() + "\".\n"
            + "Unable to split the address into host:module");
        }*/

        if(dataPackage.hasControlVal())
        {
            Logger.logError("ModuleManager received DataPackage with controlVal. The controlVal should have been handled by ModuleDispatcher");
            return;
        }

        /*String hostName = hostAndModule[0];
        String moduleId = hostAndModule[1];*/

        final String channelName = dataPackage.getChannel();
        final String moduleId = dataPackage.getTargetHostModule();

        Logger.logInfo("ModuleManager received package with target for Module \"" + moduleId + "\" on Channel \"" + channelName + "\"");

        if(this.subscriberPublisher == null)
        {
            Logger.logError("ModuleManager received DataPackage, however SubscriberPublisher is Null.");
            return;
        }

        if(!subscriberPublisher.isDataPackageCompatibleWithChannel(dataPackage))
        {
            Logger.logInfo("ModuleManager received package with target for Module \"" + moduleId + "\" on Channel \"" + channelName + "\",\n"
            + "however the data type of payload of the package did not match the data type of the Channel.\n"
            + "Expected payload type \"" + subscriberPublisher.getPayloadCaseOfChannel(channelName).name() + "\" but got \"" + dataPackage.getPayloadOneofCase().name());
            return;
        }

        ArrayList<AbstractSubscriber> subscriberList = this.subscriberPublisher.getSubscriberInstancesOfModule(channelName, moduleId);

        if(subscriberList == null)
        {
            Logger.logInfo("ModuleManager received package with target for Module \"" + moduleId + "\" on Channel \"" + channelName + "\",\n" + 
            "however a Subscriber of the Module for this Channel was not found. The Module has no Subscriber for this Channel.");
        }
        

        for(AbstractSubscriber subscriber : subscriberList)
        {
            subscriber.onNewData(dataPackage);
        }
    }

    public void readFromModules()
    {
        while(this.running)
        {
            // Todo: Check if we received the ping from the Middleware yet and are fully registered.
            System.out.println("Read from modules");
            DataPackage dataPackage = this.fromModulesChannel.blockingGet();
            if(dataPackage != null)
            {
                this.onDataPackageFromModule(dataPackage);
            }
            else
            {
                System.out.println("Read from modules null");
            }
        }
    }

    public void onDataPackageFromModule(DataPackage dataPackage)
    {
        Logger.logInfo("ModuleManager received local package from Module \"" + dataPackage.getSourceHostModule() + "\".");
        Logger.logInfo(dataPackage + " ");
        this.dispatcher.postPackage(dataPackage);
    }
  
}