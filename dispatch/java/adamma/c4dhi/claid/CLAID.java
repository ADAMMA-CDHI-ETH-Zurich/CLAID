package adamma.c4dhi.claid;

import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Module.ModuleFactory;

public class CLAID
{
    static {
        System.loadLibrary("claid_capi_java");
    }

    private ModuleDispatcher moduleDispatcher;
    private ModuleManager moduleManager;

    // Starts the middleware.
    private static native long startCore(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId);
    private static native void shutdownCore(long handle);

    public static native void sayHelloDD();
    // Starts the middleware
    public static boolean start(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        startCore(socketPath, configFilePath, hostId, userId, deviceId);
     
        if(!startAttach(socketPath, moduleFactory))
        {
            return false;
        }

        return true;
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    public static boolean startAttach(final String socketPath, ModuleFactory factory)
    {
        return false;
    }
}