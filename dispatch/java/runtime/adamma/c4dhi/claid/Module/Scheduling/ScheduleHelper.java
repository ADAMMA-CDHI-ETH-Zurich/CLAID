package adamma.c4dhi.claid.Module.Scheduling;

import java.time.Duration;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;

import adamma.c4dhi.claid.Schedule;
import adamma.c4dhi.claid.SchedulePeriodic;
import adamma.c4dhi.claid.ScheduleTimeOfDay;

public class ScheduleHelper {
    public static Duration getIntervalDurationFromPeriodicSchedule(SchedulePeriodic schedulePeriodic) {
        switch (schedulePeriodic.getIntervalCase()) {
            case FREQUENCY_HZ:
                return Duration.ofMillis((long) (1000.0 / schedulePeriodic.getFrequencyHz()));
            case FREQUENCY_KHZ:
                return Duration.ofMillis((long) (1000.0 / (schedulePeriodic.getFrequencyKHz() * 1000.0)));
            case FREQUENCY_MHZ:
                return Duration.ofMillis((long) (1000.0 / (schedulePeriodic.getFrequencyMHz() * 1000000.0)));
            case PERIOD_MILLISECONDS:
                return Duration.ofMillis((long) schedulePeriodic.getPeriodMilliseconds());
            case PERIOD_SECONDS:
                return Duration.ofSeconds((long) schedulePeriodic.getPeriodSeconds());
            case PERIOD_MINUTES:
                return Duration.ofMinutes((long) schedulePeriodic.getPeriodMinutes());
            case PERIOD_HOURS:
                return Duration.ofHours((long) schedulePeriodic.getPeriodHours());
            case PERIOD_DAYS:
                return Duration.ofDays((long) schedulePeriodic.getPeriodDays());
            default:
                return Duration.ZERO;
        }
    }

    public static LocalDateTime calculateNextTimeOfDay(ScheduleTimeOfDay timeOfDay) {
        // Get the current date and time
        LocalDateTime now = LocalDateTime.now();

        // Extract current date and time components
        LocalDate today = now.toLocalDate();

        // Extract target time components from ScheduleTimeOfDay
        int targetHour = timeOfDay.getHour();
        int targetMinute = timeOfDay.getMinute();
        int targetSecond = timeOfDay.getSecond();

        // Create a LocalTime object for the target time of day
        LocalTime targetTime = LocalTime.of(targetHour, targetMinute, targetSecond);

        // Create a LocalDateTime object for today at the target time
        LocalDateTime targetDateTime = LocalDateTime.of(today, targetTime);

        // If the target time has already passed for today, schedule it for tomorrow
        if (now.isAfter(targetDateTime)) {
            targetDateTime = targetDateTime.plusDays(1);
        }

        return targetDateTime;
    }

    public static boolean isFirstTimeOfDayBeforeSecond(ScheduleTimeOfDay first, ScheduleTimeOfDay second) {
        LocalTime firstTime = LocalTime.of(first.getHour(), first.getMinute(), first.getSecond());
        LocalTime secondTime = LocalTime.of(second.getHour(), second.getMinute(), second.getSecond());
        return firstTime.isBefore(secondTime);
    }

    public static boolean areTimesOfDayEqual(ScheduleTimeOfDay first, ScheduleTimeOfDay second) {
        return first.getHour() == second.getHour() &&
                first.getMinute() == second.getMinute() &&
                first.getSecond() == second.getSecond();
    }

    public static boolean isScheduleEmpty(Schedule schedule)
    {
        return schedule.getPeriodicList().size() == 0 && schedule.getTimedList() == null;
    }

}
