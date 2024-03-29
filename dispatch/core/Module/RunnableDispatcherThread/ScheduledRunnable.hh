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

#include "Runnable.hh"
#include "ScheduleDescription/ScheduleDescription.hh"

#include <memory>


namespace claid
{
    struct ScheduledRunnable
    {
        public:
            std::shared_ptr<Runnable> runnable;
            std::shared_ptr<ScheduleDescription> schedule;

            ScheduledRunnable()
            {

            }

            ScheduledRunnable(
                std::shared_ptr<Runnable> runnable, 
                std::shared_ptr<ScheduleDescription> schedule) : runnable(runnable), schedule(schedule)
            {

            }

            template<typename ScheduleDescriptionType>
            ScheduledRunnable(
                std::shared_ptr<Runnable> runnable,
                std::shared_ptr<ScheduleDescriptionType> schedule) : runnable(runnable)
            {
                this->schedule = 
                    std::static_pointer_cast<ScheduleDescription>(schedule);
            }

            template<typename ScheduleDescriptionType>
            ScheduledRunnable(
                std::shared_ptr<Runnable> runnable,
                ScheduleDescriptionType schedule) : runnable(runnable)
            {
                // Copy data to a shared pointer
                std::shared_ptr<ScheduleDescriptionType> tmpSchedule(new ScheduleDescriptionType(schedule));
                this->schedule = 
                    std::static_pointer_cast<ScheduleDescription>(tmpSchedule);
            }
    };
}