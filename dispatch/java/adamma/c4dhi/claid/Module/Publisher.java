package adamma.c4dhi.claid.Module;


import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;

public class Publisher<T> 
{
    private final Class<T> dataType;
    private final String moduleId;
    private final String channelName;
    private ThreadSafeChannel<DataPackage> toModuleManagerQueue;
    private Mutator<T> mutator;

    public Publisher(Class<T> dataType, final String moduleId, final String channelName, ThreadSafeChannel<DataPackage> toModuleManagerQueue)
    {
        this.dataType = dataType;
        this.moduleId = moduleId;
        this.channelName = channelName;
        this.toModuleManagerQueue = toModuleManagerQueue;

        T instance = TypeMapping.getNewInstance(dataType);
        this.mutator = TypeMapping.getMutator(instance);
    }

    public void post(T data)
    {
        DataPackage.Builder builder = DataPackage.newBuilder();
        builder.setSourceHostModule(this.moduleId);
        builder.setChannel(this.channelName);
        DataPackage dataPackage = builder.build();
        dataPackage = this.mutator.setPackagePayload(dataPackage, data);

        this.toModuleManagerQueue.add(dataPackage);
    }        
}
