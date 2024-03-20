package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid.EventTracker.EventTracker;
import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.ControlPackage;
import adamma.c4dhi.claid.CtrlType;
import adamma.c4dhi.claid.Runtime;

import adamma.c4dhi.claid.PowerProfile;

import java.util.Map;

public abstract class ManagerModule extends Module 
{

    public void pauseModuleById(String moduleId) 
    {
        DataPackage.Builder packageBuilder = DataPackage.newBuilder();
        ControlPackage.Builder ctrlPackageBuilder = ControlPackage.newBuilder();
        
        ctrlPackageBuilder.setCtrlType(CtrlType.CTRL_PAUSE_MODULE);
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_CPP);
    
        packageBuilder.setControlVal(ctrlPackageBuilder);
        packageBuilder.setTargetModule(moduleId);
        packageBuilder.setSourceModule(this.getId());

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this.subscriberPublisher.getToModuleManagerQueue().add(packageBuilder.build());
    }

    public void resumeModuleById(String moduleId) {
        DataPackage.Builder packageBuilder = DataPackage.newBuilder();
        ControlPackage.Builder ctrlPackageBuilder = ControlPackage.newBuilder();
        
        ctrlPackageBuilder.setCtrlType(CtrlType.CTRL_UNPAUSE_MODULE);
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_CPP);
    
        packageBuilder.setControlVal(ctrlPackageBuilder);
        packageBuilder.setTargetModule(moduleId);
        packageBuilder.setSourceModule(this.getId());

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this.subscriberPublisher.getToModuleManagerQueue().add(packageBuilder.build());
    }

    public void adjustPowerProfileOnModuleById(String moduleId, PowerProfile powerProfile) 
    {
        DataPackage.Builder packageBuilder = DataPackage.newBuilder();
        ControlPackage.Builder ctrlPackageBuilder = ControlPackage.newBuilder();
        
        ctrlPackageBuilder.setCtrlType(CtrlType.CTRL_ADJUST_POWER_PROFILE);
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_CPP);
        // Assuming setPowerProfile method is generated
        ctrlPackageBuilder.setPowerProfile(powerProfile);
        
        packageBuilder.setControlVal(ctrlPackageBuilder);
        packageBuilder.setTargetModule(moduleId);
        packageBuilder.setSourceModule(this.getId());

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this.subscriberPublisher.getToModuleManagerQueue().add(packageBuilder.build());
    }

    @Override
    public boolean start(ChannelSubscriberPublisher subscriberPublisher, Properties properties) 
    {
        super.start(subscriberPublisher, properties);
        this.subscriberPublisher = subscriberPublisher;
        return true;
    }
};