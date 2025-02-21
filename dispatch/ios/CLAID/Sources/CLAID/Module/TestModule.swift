//
//  TestModule.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//

actor TestModule : Module {
    private var ctr = 0
    
    func initialize() async throws {
        await registerPeriodicFunction(name: "incrementCounter", interval: 2) {
            await self.incrementCounter()
        }
    }
    
    func incrementCounter() async {
        ctr += 1
    }
}
