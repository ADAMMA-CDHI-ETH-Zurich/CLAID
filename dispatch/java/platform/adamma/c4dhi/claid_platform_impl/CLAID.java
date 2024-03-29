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

import adamma.c4dhi.claid.JavaCLAIDBase;
import adamma.c4dhi.claid.Module.ModuleFactory;

public class CLAID extends JavaCLAIDBase
{
    static
    {
        init("claid_capi_java");
    }
    
    // Starts the middleware and attaches to it.
    public static boolean start(final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        return startInternal(socketPath, configFilePath, hostId, userId, deviceId, moduleFactory);
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    public static boolean attachJavaRuntime(final String socketPath, ModuleFactory factory)
    {
        return attachJavaRuntimeInternal(socketPath, factory);
    }

    public static boolean attachJavaRuntime(long handle, ModuleFactory factory)
    {
        return attachJavaRuntimeInternal(handle, factory);
    }

    public static ModuleFactory registerDefaultModulesToFactory(ModuleFactory factory)
    {
        return factory;
    }

    // In plain Java, we simply stop the Middleware.
    // In Android, we also might have to stop the services, so this might be implemented differently
    public static void shutdown()
    {
        shutdownInternal();   
    }

}
