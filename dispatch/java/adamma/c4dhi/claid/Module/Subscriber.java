package adamma.c4dhi.claid.Module;

import java.util.function.Consumer;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Scheduling.ConsumerRunnable;
import adamma.c4dhi.claid.Module.Scheduling.RunnableDispatcher;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleOnce;
import adamma.c4dhi.claid.Module.Scheduling.ScheduledRunnable;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.DataPackage;

public class Subscriber<T extends Object> extends AbstractSubscriber
{
    private Class<T> dataType;
    private Consumer<T> callback;
    private RunnableDispatcher callbackDispatcher;

    private Mutator<T> mutator;
        
    public Subscriber(Class<T> dataType, Consumer<T> callback, RunnableDispatcher callbackDispatcher)
    {
        this.dataType = dataType;
        this.callback = callback;
        this.callbackDispatcher = callbackDispatcher;
        this.mutator = TypeMapping.getMutator(dataType);
    }

    private void invokeCallback(T data)
    {
        ConsumerRunnable<T> consumerRunnable = new ConsumerRunnable<T>(data, callback, ScheduleOnce.now());
        ScheduledRunnable runnable = (ScheduledRunnable) consumerRunnable;

        this.callbackDispatcher.addRunnable(runnable);
    }

    @Override
    public void onNewData(DataPackage data) 
    {
        T value = this.mutator.getPackagePayload(data);
        this.invokeCallback(value);

        /*if(dataType.isInstance(data))
        {   
            T typedData = dataType.cast(data);
            invokeCallback(typedData);
        }
        else
        {
            Logger.logError("Error in Java Runtime: Subscriber received data of type \"" + data.getClass().getName() + "\", but\n"
            + "expected data of type \"" + dataType.getName() + "\". Cast failed.");
        }*/
    }

}
