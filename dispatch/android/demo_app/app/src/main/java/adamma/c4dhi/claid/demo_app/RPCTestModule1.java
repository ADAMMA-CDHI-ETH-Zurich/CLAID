package adamma.c4dhi.claid.demo_app;

import java.time.Duration;
import java.time.LocalDateTime;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;
import adamma.c4dhi.claid.RemoteFunction.RemoteFunction;

public class RPCTestModule1 extends Module
{

    @Override
    protected void initialize(Properties properties) {
        registerScheduledFunction("Test", LocalDateTime.now().plusSeconds(3), () ->scheduledFunc());
        moduleInfo("RPCTestModule init");
    }

    public void scheduledFunc()
    {
        moduleInfo("Scheduled function called");

        RemoteFunction<String> func =
                mapRemoteFunctionOfModule("RPCTestModule2", "testFunction",
                        String.class, String.class);

        String result = func.execute("Test ").await();

        moduleInfo("Received result " + result);
    }

}
