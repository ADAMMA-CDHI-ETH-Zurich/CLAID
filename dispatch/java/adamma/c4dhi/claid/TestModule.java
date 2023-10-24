package adamma.c4dhi.claid;

import java.util.Map;

import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.Module;

public class TestModule extends Module
{

    private Channel<Double> testChannel;

    @Override
    public void initialize(Map<String, String> properties) 
    {
        this.testChannel = this.publish(Double.class, "TestChannel");
        System.out.println("Hello world from TestModule!");
        this.testChannel.post(Double.valueOf(42));
    }

    public void onData(Double data)
    {

    }
}