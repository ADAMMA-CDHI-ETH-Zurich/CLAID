/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

package adamma.c4dhi.claid.Module.Scheduling;

import adamma.c4dhi.claid.Schedule;
import adamma.c4dhi.claid.ScheduleTimeWindow;
import adamma.c4dhi.claid.ScheduleTimeOfDay;
import adamma.c4dhi.claid.Logger.Logger;

import java.time.Duration;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.temporal.ChronoUnit;
import java.time.temporal.TemporalUnit;

public class ScheduleRepeatedIntervall implements ScheduleDescription {
    private LocalDateTime executionTime;
    private Duration interval;

    private boolean isAlwaysActive = true;

    private ScheduleTimeWindow onlyActiveBetween = null;

    public ScheduleRepeatedIntervall(LocalDateTime startTime, Duration interval) {
        this.executionTime = startTime;
        this.interval = interval;
    }

    public ScheduleRepeatedIntervall(
        LocalDateTime startTime, 
        Duration interval, 
        ScheduleTimeWindow onlyActiveBetween
    ) 
    {
        ScheduleTimeOfDay start = onlyActiveBetween.getStartTimeOfDay();
        ScheduleTimeOfDay stop = onlyActiveBetween.getStopTimeOfDay();
        // Check if start and stop times are equal
        if (ScheduleHelper.areTimesOfDayEqual(start, stop)) {
        String message = String.format(
            "Cannot schedule function which is only active during certain times of the day; \n" +
            "the provided start and stop time are equal at %02d:%02d:%02d.",
            start.getHour(), start.getMinute(), start.getSecond()
        );
        Logger.logFatal(message);
        return;
    }
    
        this.executionTime = startTime;
        this.interval = interval;
        this.isAlwaysActive = false;
        this.onlyActiveBetween = onlyActiveBetween;
    }

    @Override
    public boolean doesRunnableHaveToBeRepeated() {
        return true;
    }

    @Override
    public final void updateExecutionTime() {
        if (!isAlwaysActive) {
            ScheduleTimeOfDay start = onlyActiveBetween.getStartTimeOfDay();
            ScheduleTimeOfDay stop = onlyActiveBetween.getStopTimeOfDay();
    
            LocalTime startTime;
            LocalTime stopTime;
    
            startTime = LocalTime.of(start.getHour(), start.getMinute(), start.getSecond());
            stopTime = LocalTime.of(stop.getHour(), stop.getMinute(), stop.getSecond());
    
            LocalDateTime startDateTime = executionTime.toLocalDate().atTime(startTime);
            LocalDateTime stopDateTime = executionTime.toLocalDate().atTime(stopTime);

            if (ScheduleHelper.isFirstTimeOfDayBeforeSecond(start, stop)) {
                // This means that the stop time is on the same day as the start time,
                // as the start time is before the stop time.
            } else {
                // Stop time is on the next day, consider the following:
                // Start time = 15:00
                // Stop time = 03:00 -> at 03:00 on the next day (otherwise it doesn't make sense).
                stopDateTime = stopDateTime.plus(1, ChronoUnit.DAYS);
            }
    
            if (!executionTime.isBefore(startDateTime) && !executionTime.isAfter(stopDateTime)) {
                executionTime = executionTime.plus(interval);
                
                if (executionTime.isAfter(stopDateTime)) {
                    startDateTime = startDateTime.plus(1, ChronoUnit.DAYS);
                    executionTime = startDateTime;
                }

            } else if (executionTime.isBefore(startDateTime)) {
                executionTime = startDateTime;
            } else if (executionTime.isAfter(stopDateTime)) {
                // If we are past the stop time, then we take the start time + 1 day as new execution time.
                startDateTime = startDateTime.plus(1, ChronoUnit.DAYS);
                executionTime = startDateTime;
            }
        } else {
            executionTime = executionTime.plus(interval);
        }
    }

    @Override
    public LocalDateTime getExecutionTime() {
        return this.executionTime;
    }
}
