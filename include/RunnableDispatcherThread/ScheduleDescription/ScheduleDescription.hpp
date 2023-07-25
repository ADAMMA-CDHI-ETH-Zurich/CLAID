#pragma once

#include "Utilities/Time.hpp"

namespace claid
{
    // Standard runnable schedule description, for runnables that do not have to be repeated.
    struct ScheduleDescription
    {
        private:
            Time executionTime;

        public:

            ScheduleDescription()
            {

            }

            ScheduleDescription(const Time& executionTime) : executionTime(executionTime)
            {

            }

            virtual bool doesRunnableHaveToBeRepeated() = 0;
            virtual void updateExecutionTime() = 0;
            virtual Time getExecutionTime() = 0;
    };
}