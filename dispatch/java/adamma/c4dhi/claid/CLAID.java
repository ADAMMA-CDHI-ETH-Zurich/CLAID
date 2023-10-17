package adamma.c4dhi.claid;

import adamma.c4dhi.claid.LocalDispatching.ModuleDispatcher;
import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Module.ModuleFactory;

public class CLAID
{
    static {
        System.loadLibrary("claid_capi");
    }

    private ModuleDispatcher moduleDispatcher;
    private ModuleManager moduleManager;

    // Starts the middleware.
    private static native void startMiddleware(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId);


    // Starts the middleware
    public boolean start(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        if(!startAttach(socketPath, moduleFactory))
        {
            return false;
        }

        startMiddleware(socketPath, configFilePath, hostId, userId, deviceId);
        return true;
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called BEFORE start is called in ANOTHER language.
    public boolean startAttach(final String socketPath, ModuleFactory factory)
    {
        return false;
    }
}