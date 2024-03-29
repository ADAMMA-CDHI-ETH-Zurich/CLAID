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
