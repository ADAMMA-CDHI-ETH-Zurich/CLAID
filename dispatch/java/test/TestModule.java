package test;

import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;


import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.TracePoint;
import adamma.c4dhi.claid.StringArray;
import adamma.c4dhi.claid.NumberArray;
import adamma.c4dhi.claid.NumberMap;
import adamma.c4dhi.claid.StringMap;
import adamma.c4dhi.claid.Module.ModuleAnnotator;

public class TestModule extends Module
{

    private Channel<NumberMap> testChannel;
    private Channel<String> testChannel2;
    private Channel<NumberMap> receiveChannel;
    private Channel<String> receiveChannel2;

    private int ctr = 0;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleDescription("Just a simple Module for testing.");
    }

    @Override
    public void initialize(Map<String, String> properties) 
    {
       this.testChannel = this.publish("TestChannel", NumberMap.class);
       this.receiveChannel = this.subscribe("TestChannel", NumberMap.class, val -> onData(val));

    //     this.receiveChannel2 = this.subscribe(String.class, "TestChannel", (data) -> onDataString(data));
    //     this.receiveChannel = this.subscribe(Double.class, "TestChannel", (data) -> onData(data));
    //     this.testChannel2 = this.publish(String.class, "TestChannel");

    //     System.out.println("Hello world from TestModule!");
    //   //  this.testChannel.post(Double.valueOf(42));
        
        
        this.registerPeriodicFunction("TestFunction", () -> testFunction(), Duration.ofMillis(500));
    }

    public void onData(ChannelData<NumberMap> data)
    {
        System.out.println("On data: " + data.getData());
    }

    public void testFunction()
    {
        Logger.logInfo("Test function hello " + ctr);
        Map<String, String> data = new HashMap<>();
        data.put("Test" + ctr, String.valueOf(ctr));

        NumberMap.Builder builder = NumberMap.newBuilder();
        builder.putVal("Test"+ctr, ctr);
        this.testChannel.post(builder.build());
        ctr += 1;

    }

    public void onDataString(String data)
    {

    }

}