package adamma.c4dhi.claid;

import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.ModuleFactory;


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
    private static native String getSocketPath(long handle);
    
    private static ModuleDispatcher moduleDispatcher;
    private static ModuleManager moduleManager;

    private static boolean started = false;
    private static long handle;

    

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

        if(!attachJavaRuntimeInternal(socketPath, moduleFactory))
        {
            return false;
        }

        started = true;

        return true;
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
        String socketPath = getSocketPath(handle);
        return attachJavaRuntimeInternal(socketPath, factory);
    }


}