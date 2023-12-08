package adamma.c4dhi.claid.Module.Scheduling;

import java.util.function.Consumer;

public class FunctionRunnable extends ScheduledRunnable
{

    final Runnable runnable;

    public FunctionRunnable(final Runnable runnable)
    {
        super(ScheduleOnce.now());
        this.runnable = runnable;
    }

    public FunctionRunnable(final Runnable runnable, ScheduleDescription schedule)
    {
        super(schedule);
        this.runnable = runnable;
    }

    @Override
    public void run() {
        runnable.run();
    }    
}
