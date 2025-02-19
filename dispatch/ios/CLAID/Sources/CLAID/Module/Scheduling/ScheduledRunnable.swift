//
//  ScheduledRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//

public protocol ScheduledRunnable: Sendable {
    var schedule: ScheduleDescription { get }
    var isValid: Bool { get }
    
    func run()
    func invalidate()
}
