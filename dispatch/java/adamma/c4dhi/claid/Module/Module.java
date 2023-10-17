package adamma.c4dhi.claid.Module;

import java.util.Map;

public abstract class Module
{
    protected String id = "unknown";


    public abstract void initialize(Map<String, String> properties);

    public void setId(String id)
    {
        this.id = id;
    }
};