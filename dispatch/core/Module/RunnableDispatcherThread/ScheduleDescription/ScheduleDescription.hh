
#pragma once

#include "dispatch/core/Utilities/Time.hh"

namespace claid
{
    // Standard runnable schedule description, for runnables that do not have to be repeated.
    struct ScheduleDescription
    {
        public:

            ScheduleDescription()
            {

            }

            virtual ~ScheduleDescription() {}

           

            virtual bool doesRunnableHaveToBeRepeated() = 0;
            virtual void updateExecutionTime() = 0;
            virtual Time getExecutionTime() = 0;
    };
}