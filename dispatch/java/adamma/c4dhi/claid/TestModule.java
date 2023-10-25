package adamma.c4dhi.claid;

import java.time.Duration;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.Module;

public class TestModule extends Module
{

    private Channel<Double> testChannel;
    private Channel<Double> receiveChannel;
    private int ctr = 0;

    @Override
    public void initialize(Map<String, String> properties) 
    {
        this.testChannel = this.publish(Double.class, "TestChannel");
        this.receiveChannel = this.subscribe(Double.class, "TestChannel", (data) -> onData(data));
        System.out.println("Hello world from TestModule!");
        this.testChannel.post(Double.valueOf(42));
        this.registerPeriodicFunction("TestFunction", () -> testFunction(), Duration.ofMillis(1));
    }

    public void onData(Double data)
    {
        System.out.println("On data: " + data);
    }

    public void testFunction()
    {
        Logger.logInfo("Test function hello " + ctr);
        this.testChannel.post(Double.valueOf(ctr));
        ctr += 1;

    }

}