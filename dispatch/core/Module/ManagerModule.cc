#include "dispatch/core/Module/ManagerModule.hh"

using namespace claidservice;
namespace claid
{
    void ManagerModule::pauseModuleById(const std::string& moduleId)
    {
        std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
        ControlPackage& ctrlPackage = *package->mutable_control_val();
        
        ctrlPackage.set_ctrl_type(CtrlType::CTRL_PAUSE_MODULE);
        ctrlPackage.set_runtime(Runtime::RUNTIME_CPP);
    
        package->set_target_module(moduleId);
        package->set_source_module(this->getId());

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this->subscriberPublisher->getToModuleDispatcherQueue().push_back(package);
    }

    void ManagerModule::resumeModuleById(const std::string& moduleId)
    {
        std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
        ControlPackage& ctrlPackage = *package->mutable_control_val();
        
        ctrlPackage.set_ctrl_type(CtrlType::CTRL_UNPAUSE_MODULE);
        ctrlPackage.set_runtime(Runtime::RUNTIME_CPP);
    
        package->set_target_module(moduleId);
        package->set_source_module(this->getId());

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this->subscriberPublisher->getToModuleDispatcherQueue().push_back(package);
    }

    void ManagerModule::adjustPowerProfileOnModuleById(const std::string& moduleId, PowerProfile powerProfile)
    {
        std::shared_ptr<DataPackage> package = std::make_shared<DataPackage>();
        ControlPackage& ctrlPackage = *package->mutable_control_val();
        
        ctrlPackage.set_ctrl_type(CtrlType::CTRL_ADJUST_POWER_PROFILE);
        ctrlPackage.set_runtime(Runtime::RUNTIME_CPP);
        *ctrlPackage.mutable_power_profile() = powerProfile;
        
        package->set_target_module(moduleId);
        package->set_source_module(this->getId());

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this->subscriberPublisher->getToModuleDispatcherQueue().push_back(package);
    }

    bool ManagerModule::start(ChannelSubscriberPublisher* subscriberPublisher, Properties properties) 
    {
        bool result = Module::start(subscriberPublisher, properties);
        this->subscriberPublisher = subscriberPublisher;   
        return result;
    }

}