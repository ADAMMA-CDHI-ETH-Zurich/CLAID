package adamma.c4dhi.claid;

import java.time.Duration;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.Module;

public class TestModule extends Module
{

    private Channel<Double> testChannel;
    private Channel<String> testChannel2;
    private Channel<Double> receiveChannel;
    private Channel<String> receiveChannel2;

    private int ctr = 0;

    @Override
    public void initialize(Map<String, String> properties) 
    {
       this.testChannel = this.publish(Double.class, "TestChanne123");
       
    //     this.receiveChannel2 = this.subscribe(String.class, "TestChannel", (data) -> onDataString(data));
    //     this.receiveChannel = this.subscribe(Double.class, "TestChannel", (data) -> onData(data));
    //     this.testChannel2 = this.publish(String.class, "TestChannel");

    //     System.out.println("Hello world from TestModule!");
    //   //  this.testChannel.post(Double.valueOf(42));
        
        
    //     this.registerPeriodicFunction("TestFunction", () -> testFunction(), Duration.ofMillis(500));
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

    public void onDataString(String data)
    {

    }

}