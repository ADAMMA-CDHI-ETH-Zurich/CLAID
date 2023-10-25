package adamma.c4dhi.claid.Module.Scheduling;

import java.time.LocalDateTime;

public class ScheduleOnce implements ScheduleDescription
{
    private LocalDateTime executionTime;


    public ScheduleOnce(LocalDateTime executionTime) 
    {
        this.executionTime = executionTime;
    }

    public static ScheduleOnce now()
    {
        return new ScheduleOnce(LocalDateTime.now());
    }

    public boolean doesRunnableHaveToBeRepeated()
    {
        return false;
    }

    public void updateExecutionTime()
    {
        // Does not exist for ScheduleOnce. 
        // A ScheduledRunnable with ScheduleDescription
        // of type ScheduleOnce is not supposed to be repeated.
    }

    public LocalDateTime getExecutionTime()
    {
        return this.executionTime;
    }
}
