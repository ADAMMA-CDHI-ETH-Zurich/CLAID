#pragma once

#include "ScheduleDescription.hpp"

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