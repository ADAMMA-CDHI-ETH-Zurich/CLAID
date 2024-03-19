public class ManagerModule extends Module 
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
    
        this.subscriberPublisher.getToModuleDispatcherQueue().add(packageBuilder.build());
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
    
        this.subscriberPublisher.getToModuleDispatcherQueue().add(packageBuilder.build());
    }

    public void adjustPowerProfileOnModuleById(String moduleId, PowerProfile powerProfile) {
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
    
        this.subscriberPublisher.getToModuleDispatcherQueue().add(packageBuilder.build());
    }

    public boolean start(ChannelSubscriberPublisher subscriberPublisher, Map<String, String> properties) 
    {
        super.start(subscriberPublisher, properties);
        this.subscriberPublisher = subscriberPublisher;
        return true;
    }
};