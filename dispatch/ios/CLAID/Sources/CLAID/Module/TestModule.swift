//
//  TestModule.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//

public actor TestModule : Module {
  
    public var moduleHandle: ModuleHandle = ModuleHandle()
    private var ctr: Int64 = 0
    private var inputChannel: Channel<Int64>?
    private var outputChannel: Channel<Int64>?
    
    public init() {
    
    }
    
    public func initialize(properties: Properties) async throws {
        
        //chan = try await self.subscribe("Test", dataTypeExample: Claidservice_NumberMap(), callback: self.onData)
        inputChannel = try await self.subscribe("InputChannel", dataTypeExample: Int64(), callback: self.onData)
        outputChannel = try await self.publish("OutputChannel", dataTypeExample: Int64())
        
        await registerPeriodicFunction(name: "TestFunc", interval: .seconds(1), function: self.count)
    }
    
    private func count() async {
        self.ctr += 1
        await self.outputChannel?.post(self.ctr)
    }
    
    private func onData(data: ChannelData<Int64>) async {
        await moduleInfo("Received value \(await data.getData())")
    }
    

    
    public func terminate() async {
        
    }
}
