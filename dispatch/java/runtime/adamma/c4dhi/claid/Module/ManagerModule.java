/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
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

package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid.EventTracker.EventTracker;
import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.ControlPackage;
import adamma.c4dhi.claid.CtrlType;
import adamma.c4dhi.claid.Runtime;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionHandler;

import adamma.c4dhi.claid.PowerProfile;

import java.util.Map;

public abstract class ManagerModule extends Module 
{

    public void pauseModuleById(String moduleId) 
    {
        DataPackage.Builder packageBuilder = DataPackage.newBuilder();
        ControlPackage.Builder ctrlPackageBuilder = ControlPackage.newBuilder();
        
        ctrlPackageBuilder.setCtrlType(CtrlType.CTRL_PAUSE_MODULE);
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_JAVA);
    
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
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_JAVA);
    
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
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_JAVA);
        // Assuming setPowerProfile method is generated
        ctrlPackageBuilder.setPowerProfile(powerProfile);
        
        packageBuilder.setControlVal(ctrlPackageBuilder);
        packageBuilder.setTargetModule(moduleId);
        packageBuilder.setSourceModule(this.getId());

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this.subscriberPublisher.getToModuleManagerQueue().add(packageBuilder.build());
    }

    public void deactivateNetworkConnections() {
        DataPackage.Builder packageBuilder = DataPackage.newBuilder();
        ControlPackage.Builder ctrlPackageBuilder = ControlPackage.newBuilder();
        
        ctrlPackageBuilder.setCtrlType(CtrlType.CTRL_DEACTIVATE_NETWORK_CONNECTIONS);
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_JAVA);
    
        packageBuilder.setControlVal(ctrlPackageBuilder);

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this.subscriberPublisher.getToModuleManagerQueue().add(packageBuilder.build());
    }

    public void activateNetworkConnections() {
        DataPackage.Builder packageBuilder = DataPackage.newBuilder();
        ControlPackage.Builder ctrlPackageBuilder = ControlPackage.newBuilder();
        
        ctrlPackageBuilder.setCtrlType(CtrlType.CTRL_ACTIVATE_NETWORK_CONNECTIONS);
        ctrlPackageBuilder.setRuntime(Runtime.RUNTIME_JAVA);
    
        packageBuilder.setControlVal(ctrlPackageBuilder);

        // Source host and target host will be set to current host 
        // automatically by Router (since this package is a control package).
    
        this.subscriberPublisher.getToModuleManagerQueue().add(packageBuilder.build());
    }

    @Override
    public boolean start(ChannelSubscriberPublisher subscriberPublisher, RemoteFunctionHandler remoteFunctionHandler, Properties properties)
    {
        super.start(subscriberPublisher, remoteFunctionHandler, properties);
        this.subscriberPublisher = subscriberPublisher;
        return true;
    }
};