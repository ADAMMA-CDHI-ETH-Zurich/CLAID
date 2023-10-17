package adamma.c4dhi.claid.LocalDispatching;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.LocalDispatching.ModuleInstanceKey;

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
public class ModuleManager 
{

    private ModuleDispatcher dispatcher;
    private ModuleFactory moduleFactory;

    private Map<ModuleInstanceKey, Module> runningModules;

    public ModuleManager(ModuleDispatcher dispatcher, ModuleFactory moduleFactory)
    {
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

        Module module = this.moduleFactory.getInstance(moduleClass);
        module.setId(moduleId);
        this.runningModules.put(new ModuleInstanceKey(moduleId, moduleClass), module);
        return true;
    }

    private boolean instantiateModules(ModuleListResponse moduleList)
    {
        for(ModuleDescriptor descriptor : moduleList.getDescriptorsList())
        {
            if(!this.instantiateModule(descriptor.getModuleId(), descriptor.getModuleClass()))
            {
                return false;
            }
        }
        return true;
    }

    private boolean initializeModules()
    {
        return false;
    }

   

    private boolean start()
    {
        ModuleListResponse moduleList = this.dispatcher.getModuleList(this.moduleFactory.getRegisteredModuleClasses());
        
        if(!instantiateModules(moduleList))
        {
            System.out.println("ModuleDispatcher: Failed to instantiate Modules.");
            return false;
        }

        if(!initializeModules())
        {
            System.out.println("Failed to initialize Modules.");
            return false;
        }

        // Map<String: moduleId, List<DataPackage>: list of channels
        Map<String, List<DataPackage>> modules;

        if(!startRuntime())
        {
            return false;
        }

        // init runtime
        // sendreceivepackafges
        // process queues

        return false;
    }

    

  
}