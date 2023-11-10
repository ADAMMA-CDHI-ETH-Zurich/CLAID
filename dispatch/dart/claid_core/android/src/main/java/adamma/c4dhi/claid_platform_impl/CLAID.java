package adamma.c4dhi.claid_platform_impl;

import adamma.c4dhi.claid.JavaCLAIDBase;
import adamma.c4dhi.claid.Module.ModuleFactory;

public class CLAID extends JavaCLAIDBase
{
    static
    {
        init("claid_capi_android");
    }
    
    // Starts the middleware and attaches to it.
    public static boolean start(final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        return startInternal(socketPath, configFilePath, hostId, userId, deviceId, moduleFactory);
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    public static boolean startAttach(final String socketPath, final String hostId, ModuleFactory factory)
    {
        return startAttachInternal(socketPath, hostId, factory);
    }

    // Implement functions for context management etc.
}
