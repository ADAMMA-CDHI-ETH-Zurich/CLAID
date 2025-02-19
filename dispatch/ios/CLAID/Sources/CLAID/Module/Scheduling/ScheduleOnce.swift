//
//  ScheduleOnce.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//


import Foundation
public actor ScheduleOnce: ScheduleDescription {
    private var executionTime: Date

    public init(executionTime: Date) {
        self.executionTime = executionTime
    }

    public static func now() -> ScheduleOnce {
        return ScheduleOnce(executionTime: Date())
    }

    public func doesRunnableHaveToBeRepeated() async -> Bool {
        return false
    }

    public func updateExecutionTime() async throws {
        // Does not exist for ScheduleOnce.
        // A ScheduledRunnable with ScheduleDescription
        // of type ScheduleOnce is not supposed to be repeated.
    }

    public func getExecutionTime()  async -> Date {
        return executionTime
    }
}
