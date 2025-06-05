//
//  TestModule.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//
 
/*
public class TestModule : Module {
     
   
    public var moduleHandle = ModuleHandle()
    public init(id: String? = nil) async {
        moduleHandle = Self.makeDefaultHandle(id: id)
    }

    public func run() async throws {
            
        inputChannel = try await self.subscribe("InputChannel", dataTypeExample: Int(), callback: self.onData)
        outputChannel = try await self.publish("OutputChannel", dataTypeExample: Int())
                        
        await outputChannel?.post(42)
    }
    
    private func onData(data: ChannelData<Int>) async {
        await moduleInfo("Received value \(await data.getData())")
    }
       
    
    private var ctr: Int = 0
    private var inputChannel: Channel<Int>?
    private var outputChannel: Channel<Int>?
  
    private func count() async {
        self.ctr += 1
        await self.outputChannel?.post(self.ctr)
    }
        
    public func terminate() async {
        
    }
}
*/
