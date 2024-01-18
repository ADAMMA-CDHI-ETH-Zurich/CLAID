package test.config_reload_test;

import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;


public class TestModule2 extends Module
{

   
    public static boolean started = false;

    @Override
    public void initialize(Map<String, String> properties) 
    {
        if(properties.containsKey("TestModule1Data"))
        {
            Integer value = Integer.parseInt(properties.get("TestModule2Data"));
            if(value == 1337)
            {
                TestModule2.started = true;
            }
        }
        else
        {
            Logger.logError("TestModule2 did not find property TestModule2Data");
        }
    }

}