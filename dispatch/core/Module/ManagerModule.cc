/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

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

    bool ManagerModule::start(ChannelSubscriberPublisher* subscriberPublisher, RemoteFunctionHandler* remoteFunctionHandler, Properties properties) 
    {
        bool result = Module::start(subscriberPublisher, remoteFunctionHandler, properties);
        this->subscriberPublisher = subscriberPublisher;   
        return result;
    }

}