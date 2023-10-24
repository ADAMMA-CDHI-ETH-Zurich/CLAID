package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.DataPackage;

public abstract class AbstractSubscriber 
{
    public abstract void onNewData(DataPackage data);
}
