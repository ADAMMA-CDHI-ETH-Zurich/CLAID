//
//  TestRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//


public class TestRunnable : ScheduledRunnable {
    let schedule = ScheduleOnce(executionTime: .now() + .seconds(1))
    let isValid: Bool = true
    
    func run() {
        
    }
    func invalidate() {
        
    }
}
