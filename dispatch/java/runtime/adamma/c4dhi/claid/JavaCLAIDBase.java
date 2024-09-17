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

package adamma.c4dhi.claid;

import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.LogMessage;

import adamma.c4dhi.claid.RemoteFunction.RemoteFunctionHandler;

public abstract class JavaCLAIDBase
{
    protected static void init(final String LIBRARY_PATH) 
    {
        if(LIBRARY_PATH == "")
        {
            Logger.logError("Java Runtime is unable to load CLAID library. LIBRARY_PATH is empty.");
            System.exit(0);
        }
        System.loadLibrary(LIBRARY_PATH);
    }
    // Starts the middleware.
    private static native long startCore(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId);
    private static native long startCoreWithEventTracker(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId, final String commonDataPath);
    private static native void shutdownCore(long handle);

    // Attaches C++ runtime to the middleware and returns a handle to the C++ runtime.
    private static native long attachCppRuntime(long handle);
    
    private static native String nativeGetSocketPath(long handle);
    private static native String nativeGetHostId(long handle);
    private static native String nativeGetUserId(long handle);
    private static native String nativeGetDeviceId(long handle);

    private static native boolean nativeLoadNewConfig(long handle, String config);

    private static native void nativeSetPayloadDataPath(long handle, String path);
    private static native String nativeGetPayloadDataPath(long handle);

    private static native void nativeSetCommonDataPath(long handle, String path);
    private static native String nativeGetCommonDataPath(long handle);
    
    private static native void nativeEnableDesignerMode(long handle);
    private static native void nativeDisableDesignerMode(long handle); 

    private static native int nativeGetLogSinkSeverityLevel(long handle);
    
    private static ModuleDispatcher moduleDispatcher;
    private static ModuleManager moduleManager;

    private static boolean started = false;
    private static long handle = 0;
    private static long cppRuntimeHandle = 0;

    private static Thread moduleManagerThread = null;
    

    // Starts the middleware and attaches to it.
    protected static boolean startInternal(final String socketPath, final String configFilePath, 
    final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        return startInternalWithEventTracker(socketPath, configFilePath, hostId, userId, deviceId, moduleFactory, new String(""));
    }

    protected static boolean startInternalWithEventTracker(final String socketPath, final String configFilePath, 
    final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory, final String commonDataPath)
    {
        if(started)
        {
            Logger.logError("CLAID middleware start was called twice!");
            return false;
        }
        
        handle = startCoreWithEventTracker(socketPath, configFilePath, hostId, userId, deviceId, commonDataPath);
        
        if(handle == 0)
        {
            Logger.logError("Failed to start CLAID middleware core (start_core), returned handle is 0.");
            return false;
        }

        cppRuntimeHandle = attachCppRuntime(handle);
        if(cppRuntimeHandle == 0)
        {
            Logger.logError("Failed to start CLAID C++ Runtime.");
            return false;
        }

        if(!attachJavaRuntimeInternal(socketPath, moduleFactory))
        {
            return false;
        }


        started = true;

        return true;
    }

    protected static void shutdownInternal()
    {
        if(handle == 0)
        {
            return;
        }

        shutdownCore(handle);
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    protected static boolean attachJavaRuntimeInternal(final String socketPath, ModuleFactory factory)
    {
        if(started)
        {
            Logger.logError("CLAID middleware start was called twice!");
            return false;
        }

        moduleDispatcher = new ModuleDispatcher(socketPath);
        moduleManager = new ModuleManager(moduleDispatcher, factory);

        if(moduleManagerThread == null)
        {
            Logger.logInfo("ModuleManagerThread 1");
            moduleManagerThread = new Thread(() -> {
                moduleManager.start();
            });
            moduleManagerThread.start();
        }
        else
        {
            
            Logger.logInfo("ModuleManagerThread 2");

            return false;
        }
            
        return true;
    }

    protected static boolean attachJavaRuntimeInternal(long handle, ModuleFactory factory)
    {
        String socketPath = getSocketPath();
        return attachJavaRuntimeInternal(socketPath, factory);
    }

    public static boolean loadNewConfig(String config)
    {
        return nativeLoadNewConfig(handle, config);
    }

    public static String getSocketPath()
    {
        return nativeGetSocketPath(handle);
    }

    public static String getHostId()
    {
        return nativeGetHostId(handle);
    }

    public static String getUserId()
    {
        return nativeGetUserId(handle);
    }

    public static String getDeviceId()
    {
        return nativeGetDeviceId(handle);
    }

    public static void setPayloadDataPath(String path)
    {
        nativeSetPayloadDataPath(handle, path);
    }

    public static String getPayloadDataPath()
    {
        return nativeGetPayloadDataPath(handle);
    }

    public static void setCommonDataPath(String path)
    {
        nativeSetCommonDataPath(handle, path);
    }

    public static String getCommonDataPath()
    {
        return nativeGetCommonDataPath(handle);
    }
    
    public static void enableDesignerMode()
    {
        Logger.logInfo("Java enable designer mode caled");
        nativeEnableDesignerMode(handle);
    }

    public static void disableDesignerMode()
    {
        nativeDisableDesignerMode(handle);
    }

    public static LogMessageSeverityLevel getLogSinkSeverityLevel()
    {
        if(handle == 0)
        {
            return LogMessageSeverityLevel.DEBUG_VERBOSE;
        }
        return LogMessageSeverityLevel.values()[nativeGetLogSinkSeverityLevel(handle)];
    }

    public static void postLogMessage(LogMessage logMessage)
    {
        if(handle == 0)
        {
            return;
        }

        moduleManager.postLogMessage(logMessage);        
    }

    public static boolean isRunning()
    {
        return JavaCLAIDBase.started;
    }

    public Module getModuleById(String moduleId)
    {
        return JavaCLAIDBase.moduleManager.getModuleById(moduleId);
    }

    public static RemoteFunctionHandler getRemoteFunctionHandler()
    {
        if(JavaCLAIDBase.moduleManager == null)
        {
            return null;
        }
        return JavaCLAIDBase.moduleManager.getRemoteFunctionHandler();
    }
}