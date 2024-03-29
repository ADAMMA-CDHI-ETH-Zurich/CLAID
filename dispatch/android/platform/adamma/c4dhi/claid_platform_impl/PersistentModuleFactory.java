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

package adamma.c4dhi.claid_platform_impl;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleFactory;

import android.app.Application;


import adamma.c4dhi.claid_platform_impl.CLAID;
public class PersistentModuleFactory extends ModuleFactory {

    private Application application;

    public <T extends Application> PersistentModuleFactory(T application) throws IllegalArgumentException
    {
        super();
        if (!(application instanceof Application)) {
            // Handle the case where application is not an instance of Application
            CLAID.onUnrecoverableException("The provided object is not an instance of Application.");
        }

        if (application.getClass().getName().equals(Application.class.getName())) {
            // Handle the case where application is an instance of the standard Application class
            CLAID.onUnrecoverableException("Cannot create PersistentModuleFactory using the standard Application class.\n"
                    + "You need to create your own Application class inheriting from Application in order to create a PersistentModuleFactory.\n"
                    + "You have to populate the PersistentModuleFactory inside your Application's onCreate function for the PersistentModuleFactory to be persistent.");
        }

        this.application = application;

        CLAID.registerDefaultModulesToFactory((ModuleFactory) this);
    }

    public Application getApplication()
    {
        return this.application;
    }

   
}
