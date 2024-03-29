
#pragma once

#include "ScheduleDescription.hh"
#include "dispatch/core/Logger/Logger.hh"

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
        
        virtual ~ScheduleRepeatedIntervall() {};

        bool doesRunnableHaveToBeRepeated() override final
        {
            return true;
        }

        void updateExecutionTime() override final
        {
            // Does not exist for ScheduleOnce. 
            // A ScheduledRunnable with ScheduleDescription
            // of type ScheduleOnce is not supposed to be repeated.
            executionTime += intervall;
            // std::cout << executionTime.strftime("Next schedule %H:%M:%S\n") << "\n";
        }

        Time getExecutionTime() override final
        {
            return this->executionTime;
        }
    };
}