//
//  TestModule.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//

public actor TestModule : @preconcurrency Module {

    
    
    public var moduleHandle: ModuleHandle = ModuleHandle()
    
    public init() {
    
    }
    
    public func initialize() async {
        await registerPeriodicFunction(name: "incrementCounter", interval: 2) {
            await self.incrementCounter()
        }
    }
    
    private var ctr = 0
    
  
    
    func incrementCounter() async {
        ctr += 1
    }
}
