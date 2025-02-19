import Foundation

public actor ScheduleRepeatedInterval: ScheduleDescription {

    
    private var executionTime: Date
    private var interval: TimeInterval
    private var isAlwaysActive: Bool = true
    private var onlyActiveBetween: Claidservice_ScheduleTimeWindow?

    public init(startTime: Date, interval: TimeInterval) {
        self.executionTime = startTime
        self.interval = interval
    }

    public init(startTime: Date, interval: TimeInterval, onlyActiveBetween: Claidservice_ScheduleTimeWindow) throws {
        let start = onlyActiveBetween.startTimeOfDay
        let stop = onlyActiveBetween.stopTimeOfDay

        if ScheduleHelper.areTimesOfDayEqual(start, stop) {
            throw CLAIDError(
                String(format: """
                    Cannot schedule function which is only active during certain times of the day; \
                    the provided start and stop time are equal at %02d:%02d:%02d.
                    """, start.hour, start.minute, start.second)
            )
        }

        self.executionTime = startTime
        self.interval = interval
        self.isAlwaysActive = false
        self.onlyActiveBetween = onlyActiveBetween
    }

    public func doesRunnableHaveToBeRepeated() async -> Bool {
        return true
    }

    public func updateExecutionTime() async throws {
        guard !isAlwaysActive, let timeWindow = onlyActiveBetween else {
            executionTime = executionTime.advanced(by: interval)
            return
        }

        let calendar = Calendar.current
        let now = Date()

        // Convert time window into real Dates for today
        guard let startTimeToday = calendar.date(from: DateComponents(
            hour: Int(timeWindow.startTimeOfDay.hour),
            minute: Int(timeWindow.startTimeOfDay.minute),
            second: Int(timeWindow.startTimeOfDay.second)
        )), let stopTimeToday = calendar.date(from: DateComponents(
            hour: Int(timeWindow.stopTimeOfDay.hour),
            minute: Int(timeWindow.stopTimeOfDay.minute),
            second: Int(timeWindow.stopTimeOfDay.second)
        )) else {
            throw CLAIDError("Invalid time window configuration: \(timeWindow.startTimeOfDay), \(timeWindow.stopTimeOfDay)")
        }

        var stopTime = stopTimeToday
        if try !ScheduleHelper.isFirstTimeOfDayBeforeSecond(timeWindow.startTimeOfDay, timeWindow.stopTimeOfDay) {
            stopTime = calendar.date(byAdding: .day, value: 1, to: stopTimeToday)!
        }

        // Update execution time while keeping it inside the allowed window
        if executionTime >= startTimeToday && executionTime <= stopTime {
            executionTime = executionTime.advanced(by: interval)

            // If execution time moves beyond stopTime, push it to the next day's start time
            if executionTime > stopTime {
                executionTime = calendar.date(byAdding: .day, value: 1, to: startTimeToday)!
            }
        } else if executionTime < startTimeToday {
            executionTime = startTimeToday
        } else {
            executionTime = calendar.date(byAdding: .day, value: 1, to: startTimeToday)!
        }
    }

    public func getExecutionTime() async -> Date {
        return executionTime
    }
}
