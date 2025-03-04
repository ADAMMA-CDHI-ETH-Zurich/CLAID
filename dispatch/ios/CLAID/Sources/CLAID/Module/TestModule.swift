//
//  TestModule.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//
 
public actor TestModule : Module {
  
    public func initialize(properties: Properties) async throws {
        
        inputChannel = try await self.subscribe("InputChannel", dataTypeExample: Int(), callback: self.onData)
        outputChannel = try await self.publish("OutputChannel", dataTypeExample: Int())
                        
        await outputChannel?.post(42)
    }
    
    private func onData(data: ChannelData<Int>) async {
        await moduleInfo("Received value \(await data.getData())")
    }
    
    //         let result: String = try await function(42, "Hello")

    
    public var moduleHandle: ModuleHandle = ModuleHandle()
    private var ctr: Int = 0
    private var inputChannel: Channel<Int>?
    private var outputChannel: Channel<Int>?
    
    public init() {
    
    }
    
    private func count() async {
        self.ctr += 1
        await self.outputChannel?.post(self.ctr)
    }
    

    

    
    public func terminate() async {
        
    }
}
