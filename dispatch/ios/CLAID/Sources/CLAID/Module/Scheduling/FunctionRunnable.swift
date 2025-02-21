//
//  FunctionRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 20.02.2025.
//


public final actor FunctionRunnable: ScheduledRunnable {
    
    public let schedule: ScheduleDescription
    private let runnable: @Sendable () async -> Void
    private var valid: Bool = true
    
    public func isValid() async -> Bool {
        return valid
    }
    
    public  init(schedule: ScheduleDescription, runnable: @Sendable @escaping () async -> Void) {
        self.schedule = schedule
        self.runnable = runnable
    }
    

    public func run() async {
        guard await isValid() else { return }
        await runnable()
    }
    
    public func invalidate() async {
        valid = false
    }
}
