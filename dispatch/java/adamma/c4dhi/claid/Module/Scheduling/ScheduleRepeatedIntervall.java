package adamma.c4dhi.claid.Module.Scheduling;

import java.time.Duration;
import java.time.LocalDateTime;
import java.time.temporal.ChronoUnit;
import java.time.temporal.TemporalUnit;

public class ScheduleRepeatedIntervall implements ScheduleDescription {
    private LocalDateTime executionTime;
    private Duration interval;

    public ScheduleRepeatedIntervall(LocalDateTime startTime, Duration interval) {
        this.executionTime = startTime;
        this.interval = interval;
    }

    @Override
    public boolean doesRunnableHaveToBeRepeated() {
        return true;
    }

    @Override
    public void updateExecutionTime() {
        executionTime = executionTime.plus(interval); // interval.toMillis(), ChronoUnit.MILLIS
    }

    @Override
    public LocalDateTime getExecutionTime() {
        return this.executionTime;
    }
}
