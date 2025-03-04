//
//  AbstractFuture.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

import Foundation

public protocol AbstractFuture : Actor {
    
    var futureInfoStore: FutureInfoStore { get }
    var futureTable: FuturesTable { get }
        
    func setResponse(_ response: Claidservice_DataPackage) async
    
    func wasExecutedSuccessfully() async -> Bool
    func awaitResponse() async -> Claidservice_DataPackage?
    func awaitResponse(timeoutDuration: Duration) async -> Claidservice_DataPackage?
    func getUniqueIdentifier() async -> FutureUniqueIdentifier
}

extension AbstractFuture {
   
    public func awaitResponse() async -> Claidservice_DataPackage? {

        let res =  await awaitResponse(timeoutDuration: Duration.seconds(Int.max))
        return res

    }
    
    public func awaitResponse(timeoutDuration: Duration) async -> Claidservice_DataPackage?  {
        
        let timeoutTime = Date.now.advanced(by: timeoutDuration.timeInterval)

        while await !futureInfoStore.finished {

            if Date.now > timeoutTime {
                await futureInfoStore.setSuccessful(false)
            }

        }

        return await futureInfoStore.successful ? await futureInfoStore.responsePackage : nil
    }
    
    public func thenUntyped(callback: @escaping @Sendable (Claidservice_DataPackage?) async -> Void) async {
        await futureInfoStore.setCallback(callback: callback)
    }
    
    public func setResponse(_ response: Claidservice_DataPackage) async {

        await futureInfoStore.setSuccessful(true)
        await futureInfoStore.setResponsePackage(response)
        await futureInfoStore.callCallbackIfSet()
        await futureInfoStore.setFinished(true)
    }
    
    public func setFailed() async {
        await futureInfoStore.unsetResponsePackage()
        await futureInfoStore.setSuccessful(false)
        await futureInfoStore.callCallbackIfSet()
        await futureInfoStore.setFinished(true)
    }
    
    public func getUniqueIdentifier() async -> FutureUniqueIdentifier  {
        return await futureInfoStore.futureUniqueIdentifier
    }
    
    public func wasExecutedSuccessfully() async -> Bool {
        let successful = await futureInfoStore.successful
        let finished = await futureInfoStore.finished
        
        return successful && finished
    }
}
