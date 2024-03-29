
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