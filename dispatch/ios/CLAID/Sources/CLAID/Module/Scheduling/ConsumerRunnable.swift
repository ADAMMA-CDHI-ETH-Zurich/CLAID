//
//  ConsumerRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//


import Foundation

public final actor ConsumerRunnable<T>: ScheduledRunnable {
    private let data: T
    private let consumer: (T) -> Void
    private var valid: Bool = true

    public let schedule: ScheduleDescription

    
    public func isValid() async -> Bool {
        return valid
    }
    

    init(data: T, consumer: @escaping (T) -> Void) {
        self.schedule = ScheduleOnce.now()
        self.data = data
        self.consumer = consumer
    }

    init(data: T, consumer: @escaping (T) -> Void, schedule: ScheduleDescription) {
        self.schedule = schedule
        self.data = data
        self.consumer = consumer
    }

    public func run() async {
        guard await isValid() else { return }
        await consumer(data)
    }
    
    public func invalidate() async {
        valid = false
    }
}
