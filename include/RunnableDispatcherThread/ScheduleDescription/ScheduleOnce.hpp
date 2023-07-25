#pragma once

#include "ScheduleDescription/ScheduleDescription.hpp"

namespace claid
{
    struct ScheduleOnce : public ScheduleDescription
    {
        ScheduleOnce(const Time& executionTime) : ScheduleDescription(executionTime)
        {

        }

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