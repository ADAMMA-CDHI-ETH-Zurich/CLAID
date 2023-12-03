package adamma.c4dhi.claid.Module.Scheduling;

public abstract class ScheduledRunnable
{
    protected ScheduleDescription schedule;
    protected boolean valid = false;

    public ScheduledRunnable(ScheduleDescription schedule)
    {
        this.schedule = schedule;
        this.valid = true;
    }

    public abstract void run();

    public ScheduleDescription getSchedule()
    {
        return this.schedule;
    }

    public boolean isValid()
    {
        return this.valid;
    }

    public void invalidate()
    {
        this.valid = false;
    }
    
}
