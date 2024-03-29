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

namespace claid
{
    struct ScheduleOnce : public ScheduleDescription
    {
        private:
            Time executionTime;

        public:

        ScheduleOnce(const Time& executionTime) : executionTime(executionTime)
        {

        }

        virtual ~ScheduleOnce() {}

        bool doesRunnableHaveToBeRepeated()
        {
            return false;
        }

        void updateExecutionTime()
        {
            // Does not exist for ScheduleOnce. 
            // A ScheduledRunnable with ScheduleDescription
            // of type ScheduleOnce is not supposed to be repeated.
        }

        Time getExecutionTime()
        {
            return this->executionTime;
        }
    };
}