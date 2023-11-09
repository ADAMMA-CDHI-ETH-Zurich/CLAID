package adamma.c4dhi.claid;

import java.time.Duration;
import java.util.ArrayList;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.Module;

public class TestModule extends Module
{

    private Channel<ArrayList<Double>> testChannel;
    private Channel<String> testChannel2;
    private Channel<ArrayList<Double>> receiveChannel;
    private Channel<String> receiveChannel2;

    private int ctr = 0;

    @Override
    public void initialize(Map<String, String> properties) 
    {
       this.testChannel = this.publish(new ArrayList<Double>(), "TestChannel");
       this.receiveChannel = this.subscribe(new ArrayList<Double>(), "TestChannel", (data) -> onData(data));

    //     this.receiveChannel2 = this.subscribe(String.class, "TestChannel", (data) -> onDataString(data));
    //     this.receiveChannel = this.subscribe(Double.class, "TestChannel", (data) -> onData(data));
    //     this.testChannel2 = this.publish(String.class, "TestChannel");

    //     System.out.println("Hello world from TestModule!");
    //   //  this.testChannel.post(Double.valueOf(42));
        
        
         this.registerPeriodicFunction("TestFunction", () -> testFunction(), Duration.ofMillis(500));
    }

    public void onData(ArrayList<Double> data)
    {
        System.out.println("On data: " + data);
    }

    public void testFunction()
    {
        Logger.logInfo("Test function hello " + ctr);
        ArrayList<Double> val = new ArrayList<>();
        val.add(Double.valueOf(ctr));
        this.testChannel.post(val);
        ctr += 1;

    }

    public void onDataString(String data)
    {

    }

}