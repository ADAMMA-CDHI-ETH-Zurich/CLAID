//
//  ScheduledRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//

public protocol ScheduledRunnable: Sendable {
    var schedule: ScheduleDescription { get }
    func isValid() async -> Bool
    
    func run() async
    func invalidate() async
    
}
