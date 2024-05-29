package adamma.c4dhi.claid.demo_app;

import java.time.Duration;
import java.time.LocalDateTime;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunction;

public class RPCTestModule2 extends Module
{

    @Override
    protected void initialize(Properties properties) {
        registerRemoteFunction("testFunction", String.class, String.class);
        moduleInfo(" init");
    }


    public String testFunction(String value)
    {
        moduleInfo("Testfunction called with value " + value);
        return value + " response42";
    }
}
