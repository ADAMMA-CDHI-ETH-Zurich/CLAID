package adamma.c4dhi.claid;

import java.util.Map;

import adamma.c4dhi.claid.Module.Module;

public class TestModule extends Module
{

    

    @Override
    public void initialize(Map<String, String> properties) 
    {
        System.out.println("Hello world from TestModule!");
    }
}