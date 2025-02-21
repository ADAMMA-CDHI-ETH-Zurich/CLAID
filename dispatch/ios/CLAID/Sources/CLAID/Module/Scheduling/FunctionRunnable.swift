//
//  FunctionRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 20.02.2025.
//


public final actor FunctionRunnable: ScheduledRunnable {
    
    public let schedule: ScheduleDescription
    private let runnable: () -> Void
    private var valid: Bool = true
    
    public func isValid() async -> Bool {
        return valid
    }
    
    public  init(schedule: ScheduleDescription, runnable: @escaping () -> Void) {
        self.schedule = schedule
        self.runnable = runnable
    }
    

    public func run() async {
        guard await isValid() else { return }
        runnable()
    }
    
    public func invalidate() async {
        valid = false
    }
}
