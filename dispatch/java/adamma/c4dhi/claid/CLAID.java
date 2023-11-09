package adamma.c4dhi.claid;

import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.ModuleFactory;

public class CLAID
{
    static {
        System.loadLibrary("claid_capi_java");
    }

    private static ModuleDispatcher moduleDispatcher;
    private static ModuleManager moduleManager;

    private static boolean started = false;
    private static long handle;

    // Starts the middleware.
    private static native long startCore(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId);
    private static native void shutdownCore(long handle);

    // Starts the middleware
    public static boolean start(final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
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

        if(!startAttach(socketPath, hostId, moduleFactory))
        {
            return false;
        }

        started = true;

        return true;
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    public static boolean startAttach(final String socketPath, final String hostId, ModuleFactory factory)
    {
        if(started)
        {
            Logger.logError("CLAID middleware start was called twice!");
            return false;
        }

        moduleDispatcher = new ModuleDispatcher(socketPath);
        moduleManager = new ModuleManager(hostId, moduleDispatcher, factory);

        return moduleManager.start();
    }
}