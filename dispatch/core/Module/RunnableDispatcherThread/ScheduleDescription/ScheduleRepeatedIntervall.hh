/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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


#pragma once

#include "ScheduleDescription.hh"
#include "dispatch/core/Logger/Logger.hh"
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Utilities/ScheduleHelper.hh"

using claidservice::ScheduleTimeWindow;

namespace claid
{
    struct ScheduleRepeatedIntervall : public ScheduleDescription
    {
        private:
            Time executionTime;
            Duration intervall;
            ScheduleTimeWindow onlyActiveBetween;

            bool isAlwaysActive = true;

        public:

        ScheduleRepeatedIntervall(const Time& startTime, Duration intervall) : 
        executionTime(startTime), intervall(intervall), isAlwaysActive(true)
        {
            
        }

        ScheduleRepeatedIntervall(
            const Time& startTime, 
            const Duration& intervall, 
            const ScheduleTimeWindow& onlyActiveBetween
        ) : executionTime(startTime), intervall(intervall), onlyActiveBetween(onlyActiveBetween), isAlwaysActive(false)
        {
            if(ScheduleHelper::areTimesOfDayEqual(onlyActiveBetween.start_time_of_day(), onlyActiveBetween.stop_time_of_day()))
            {
                Logger::logFatal(
                    "Cannot schedule function which is only active during certain times of the day; \n"
                    "the provided start and stop time are equal at %02d:%02d:%02d.",
                    onlyActiveBetween.start_time_of_day().hour(),
                    onlyActiveBetween.start_time_of_day().minute(),
                    onlyActiveBetween.start_time_of_day().second()
                );
            }
        }
        
        virtual ~ScheduleRepeatedIntervall() {};

        bool doesRunnableHaveToBeRepeated() override final
        {
            return true;
        }

        void updateExecutionTime() override final
        {
            if(!isAlwaysActive)
            {
                const ScheduleTimeOfDay& start = onlyActiveBetween.start_time_of_day();
                const ScheduleTimeOfDay& stop = onlyActiveBetween.stop_time_of_day();
                
                Time startTime;
                Time stopTime;

                startTime = Time::todayAt(start.hour(), start.minute(), start.second());
                stopTime = Time::todayAt(stop.hour(), stop.minute(), stop.second());
                if(ScheduleHelper::isFirstTimeOfDayBeforeSecond(start, stop))
                {
                    // This means that the stop time is on the same day as the start time,
                    // as the start time is before the stop time.
                }
                else
                {
                    // Stop time is on the next day, consider the following:
                    // Start time = 15:00
                    // Stop time = 03:00 -> at 03:00 on the next day (otherwise it doesn't make sense).
                    stopTime += Duration::days(1);
                }

                if(executionTime >= startTime && executionTime <= stopTime)
                {
                    executionTime += intervall;
                }
                else if(executionTime < startTime)
                {
                    executionTime = startTime;
                }
                else if(executionTime > stopTime)
                {
                    // If we are past the stop time, then we take the start time + 1 day as new execution time.
                    startTime += Duration::days(1);
                    executionTime = startTime;
                }
            }
            else
            {
                executionTime += intervall;
            }
        }

        Time getExecutionTime() override final
        {
            return this->executionTime;
        }
    };
}