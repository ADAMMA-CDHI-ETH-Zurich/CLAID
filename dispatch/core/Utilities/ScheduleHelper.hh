#pragma once

#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Utilities/Time.hh"
using claidservice::Schedule;
using claidservice::SchedulePeriodic;
using claidservice::ScheduleTimeOfDay;

namespace claid {
class ScheduleHelper
{
public:
    static Duration getIntervalDurationFromPeriodicSchedule(const SchedulePeriodic schedule)
    {
        switch(schedule.interval_case())
        {
            case SchedulePeriodic::IntervalCase::kFrequencyHz: {
                return Duration::milliseconds(1000/schedule.frequency_hz());
            }
            case SchedulePeriodic::IntervalCase::kFrequencyKHz: {
                return Duration::milliseconds(1000/(schedule.frequency_khz()*1000));
            }
            case SchedulePeriodic::IntervalCase::kFrequencyMHz: {
                return Duration::milliseconds(1000/(schedule.frequency_mhz()*1000000));
            }
            case SchedulePeriodic::IntervalCase::kPeriodMilliseconds: {
                return Duration::milliseconds(schedule.period_milliseconds());
            }
            case SchedulePeriodic::IntervalCase::kPeriodSeconds: {
                return Duration::milliseconds(schedule.period_milliseconds()*1000);
            }
            case SchedulePeriodic::IntervalCase::kPeriodMinutes: {
                return Duration::milliseconds(schedule.period_milliseconds()*1000*60);
            }
            case SchedulePeriodic::IntervalCase::kPeriodHours: {
                return Duration::milliseconds(schedule.period_milliseconds()*1000*60*60);
            }
            case SchedulePeriodic::IntervalCase::kPeriodDays: {
                return Duration::milliseconds(schedule.period_milliseconds()*1000*60*60*24);
            }
            default: {
                return Duration::milliseconds(0);
            }
        }
    }

    /// @brief Calculates the next execution time either today or tomorrow based on a time of day. 
    /// The returned time will be on the current day, if the current time of day is before specified hour, minute and second, and on the next day otherwise.
    /// 
    /// @param timeOfDay A time (hour, minute, second) of the day.
    /// @return A Time either on the current day or the next day with the specified hour, minute and second as provided by timeOfDay.
    static Time calculateNextTimeOfDay(const ScheduleTimeOfDay& timeOfDay)
    {
        Time time = Time::now();
        int year, month, day, hour, minute, second;

        time.toDateTime(year, month, day, hour, minute, second);

        if(hour * 60 * 60 + minute * 60 + second >= timeOfDay.hour() * 60 * 60 + timeOfDay.minute() * 60 + timeOfDay.second())
        {
            time += Duration::days(1);
        }
        else
        {
            time = Time::todayAt(hour, minute, second);
        }
        return time;
    }
};
}