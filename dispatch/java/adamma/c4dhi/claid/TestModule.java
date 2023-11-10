package adamma.c4dhi.claid;

import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.Module;

public class TestModule extends Module
{

    private Channel<Map<String, String>> testChannel;
    private Channel<String> testChannel2;
    private Channel<Map<String, String>> receiveChannel;
    private Channel<String> receiveChannel2;

    private int ctr = 0;

    @Override
    public void initialize(Map<String, String> properties) 
    {
       this.testChannel = this.publish("TestChannel", new HashMap<String, String>());
       this.receiveChannel = this.subscribe("TestChannel", new HashMap<String,String>(), (data) -> onData(data));

    //     this.receiveChannel2 = this.subscribe(String.class, "TestChannel", (data) -> onDataString(data));
    //     this.receiveChannel = this.subscribe(Double.class, "TestChannel", (data) -> onData(data));
    //     this.testChannel2 = this.publish(String.class, "TestChannel");

    //     System.out.println("Hello world from TestModule!");
    //   //  this.testChannel.post(Double.valueOf(42));
        
        
         this.registerPeriodicFunction("TestFunction", () -> testFunction(), Duration.ofMillis(500));
    }

    public void onData(Map<String, String> data)
    {
        System.out.println("On data: " + data);
    }

    public void testFunction()
    {
        Logger.logInfo("Test function hello " + ctr);
        Map<String, String> data = new HashMap<>();
        data.put("Test" + ctr, String.valueOf(ctr));
        this.testChannel.post(data);
        ctr += 1;

    }

    public void onDataString(String data)
    {

    }

}