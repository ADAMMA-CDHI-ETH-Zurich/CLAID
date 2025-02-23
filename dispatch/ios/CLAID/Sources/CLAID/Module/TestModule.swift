//
//  TestModule.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//

public actor TestModule : Module {
  
    public var moduleHandle: ModuleHandle = ModuleHandle()
    private var ctr: Int64 = 0
    private var chan: Channel<Claidservice_NumberMap>?
    private var outputChannel: Channel<Int64>?
    
    public init() {
    
    }
    
    public func initialize(properties: Properties) async throws {
        
        //chan = try await self.subscribe("Test", dataTypeExample: Claidservice_NumberMap(), callback: self.onData)
        outputChannel = try await self.publish("OutputChannel", dataTypeExample: Int64())
        
        await registerPeriodicFunction(name: "TestFunc", interval: Duration.milliseconds(1000), function: self.count)
    }
    
    private func count() async {
        
        await moduleInfo("Counter is \(self.ctr)")
        self.ctr += 1
        await self.outputChannel?.post(self.ctr)
    }
    
    private func onData(data: ChannelData<Claidservice_NumberMap>) {
        
    }
    

    
    public func terminate() async {
        
    }
}
