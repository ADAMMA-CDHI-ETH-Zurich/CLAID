//
//  TestModule2.swift
//  CLAID
//
//  Created by Patrick Langer on 26.02.2025.
//


public actor TestModule2 : Module {
    
    public func initialize(properties: Properties) async throws {
        try await registerRemoteFunction(
            functionName: "test_function",
            returnTypeExample: String(),
            Int(), String() , function: self.testFunction)
    }
    
    public init() {
        
    }
    
    public var moduleHandle = ModuleHandle()
    
    public func testFunction(intVal: Int, stringVal: String) -> String {
        return "TestModule2 says: \(intVal), \(stringVal)"
    }
    
    public func terminate() async {
        
    }
}
