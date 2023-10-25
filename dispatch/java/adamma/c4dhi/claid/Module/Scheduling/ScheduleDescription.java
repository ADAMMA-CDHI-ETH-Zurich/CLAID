package adamma.c4dhi.claid.Module.Scheduling;

import java.time.LocalDateTime;


public interface ScheduleDescription 
{
    boolean doesRunnableHaveToBeRepeated();
    void updateExecutionTime();
    LocalDateTime getExecutionTime();
}
    

