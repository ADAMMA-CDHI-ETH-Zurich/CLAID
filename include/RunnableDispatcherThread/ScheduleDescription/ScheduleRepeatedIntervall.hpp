#pragma once

#include "ScheduleDescription.hpp"

namespace claid
{
    struct ScheduleRepeatedIntervall : public ScheduleDescription
    {
        private:
            Time executionTime;
            Duration intervall;

        public:

        ScheduleRepeatedIntervall(const Time& startTime, Duration intervall) : executionTime(startTime), intervall(intervall)
        {

        }

        bool doesRunnableHaveToBeRepeated()
        {
            return true;
        }

        void updateExecutionTime()
        {
            // Does not exist for ScheduleOnce. 
            // A ScheduledRunnable with ScheduleDescription
            // of type ScheduleOnce is not supposed to be repeated.
            executionTime += intervall;
        }

        Time getExecutionTime()
        {
            return this->executionTime;
        }
    };
}