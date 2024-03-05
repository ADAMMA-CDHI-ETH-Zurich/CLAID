package adamma.c4dhi.claid;

import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.LogMessage;


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
    
    private static native void nativeEnableDesignerMode(long handle);
    private static native void nativeDisableDesignerMode(long handle); 

    private static native int nativeGetLogSinkSeverityLevel(long handle);
    
    private static ModuleDispatcher moduleDispatcher;
    private static ModuleManager moduleManager;

    private static boolean started = false;
    private static long handle = 0;
    private static long cppRuntimeHandle = 0;

    

    // Starts the middleware and attaches to it.
    protected static boolean startInternal(final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        if(started)
        {
            Logger.logError("CLAID middleware start was called twice!");
            return false;
        }
        
        handle = startCore(socketPath, configFilePath, hostId, userId, deviceId);
        
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

    protected static void shutdown()
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

        return moduleManager.start();
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
}