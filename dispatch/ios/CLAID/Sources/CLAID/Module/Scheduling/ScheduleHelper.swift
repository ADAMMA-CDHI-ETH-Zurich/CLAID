//
//  ScheduleHelper.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//

import Foundation

public class ScheduleHelper {
    

    public static func getIntervalDurationFromPeriodicSchedule(_ schedulePeriodic: Claidservice_SchedulePeriodic) -> Duration {
        switch schedulePeriodic.interval {
        case .frequencyHz:
            return Duration.milliseconds(Int64(1000.0 / schedulePeriodic.frequencyHz))
        case .frequencyKHz:
            return Duration.milliseconds(Int64(1000.0 / (schedulePeriodic.frequencyKHz * 1000.0)))
        case .frequencyMhz:
            return Duration.milliseconds(Int64(1000.0 / (schedulePeriodic.frequencyMhz * 1_000_000.0)))
        case .periodMilliseconds:
            return Duration.milliseconds(Int64(schedulePeriodic.periodMilliseconds))
        case .periodSeconds:
            return Duration.seconds(Int64(schedulePeriodic.periodSeconds))
        case .periodMinutes:
            return Duration.seconds(Int64(schedulePeriodic.periodMinutes * 60))
        case .periodHours:
            return Duration.seconds(Int64(schedulePeriodic.periodHours * 60 * 60))
        case .periodDays:
            return Duration.seconds(Int64(schedulePeriodic.periodDays * 60 * 60 * 24)) // Duration doesn't have a "days" unit, so convert to hours
        default:
            return .zero
        }
    }

    public static func calculateNextTimeOfDay(_ timeOfDay: Claidservice_ScheduleTimeOfDay) -> Date {
        let now = Date()
        let calendar = Calendar.current
        
        var dateComponents = calendar.dateComponents([.year, .month, .day], from: now)
        dateComponents.hour = Int(Int32(timeOfDay.hour))
        dateComponents.minute = Int(Int32(timeOfDay.minute))
        dateComponents.second = Int(Int32(timeOfDay.second))
        
        if let targetDateTime = calendar.date(from: dateComponents), targetDateTime > now {
            return targetDateTime
        }
        
        return calendar.date(byAdding: .day, value: 1, to: dateComponents.date!)!
    }

    public static func isFirstTimeOfDayBeforeSecond(_ first: Claidservice_ScheduleTimeOfDay, _ second: Claidservice_ScheduleTimeOfDay) throws -> Bool {
        guard let firstTime = Calendar.current.date(from: DateComponents(
            hour: Int(first.hour),
            minute: Int(first.minute),
            second: Int(first.second)
        )), let secondTime = Calendar.current.date(from: DateComponents(
            hour: Int(second.hour),
            minute: Int(second.minute),
            second: Int(second.second)
        )) else {
            throw CLAIDError("Invalid dates specified in config for schedule: \(first), \(second)")
        }
        return firstTime < secondTime
    }

    public static func areTimesOfDayEqual(_ first: Claidservice_ScheduleTimeOfDay, _ second: Claidservice_ScheduleTimeOfDay) -> Bool {
        return first.hour == second.hour &&
               first.minute == second.minute &&
               first.second == second.second
    }


    public static func isScheduleEmpty(_ schedule: Claidservice_Schedule) -> Bool {
        return schedule.periodic.isEmpty && schedule.timed.isEmpty
    }
}
