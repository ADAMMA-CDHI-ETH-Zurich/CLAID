//
//  AbstractFuture.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

import Foundation

protocol AbstractFuture : Actor {
    
    var futureInfoStore: FutureInfoStore { get }
    var futureTable: FuturesTable { get }
    
    init(futuresTable: FuturesTable, uniqueIdentifier: FutureUniqueIdentifier)
    
    func setResponse(_ response: Claidservice_DataPackage)
    func setFailed()
    
    func wasExecutedSuccessfully() -> Bool
    func awaitResponse() -> Claidservice_DataPackage
    func awaitResponse(timeoutDuration: Duration) throws -> Claidservice_DataPackage
    func getUniqueIdentifier() -> FutureUniqueIdentifier
}

extension AbstractFuture {
   
    
    func awaitResponse(timeoutDuration: Duration) async -> Claidservice_DataPackage?  {
        
        let timeoutTime = Date.now.advanced(by: timeoutDuration.timeInterval)
                
        while await !futureInfoStore.finished {
            if Date.now > timeoutTime {
                await futureInfoStore.setSuccessful(false)
            }
        }
        
        return await futureInfoStore.successful ? await futureInfoStore.responsePackage : nil
    }
    
    func thenUntyped(callback: @escaping (Claidservice_DataPackage?) -> Void) async {
        await futureInfoStore.setCallback(callback: callback)
    }
    
    func setResponse(responsePackage: Claidservice_DataPackage) async {
        await futureInfoStore.setResponsePackage(responsePackage)
        await futureInfoStore.setSuccessful(true)
        await futureInfoStore.setResponsePackage(responsePackage)
        await futureInfoStore.callCallbackIfSet()
        await futureInfoStore.setSuccessful(true)
    }
    
    func setFailed() async {
        await futureInfoStore.unsetResponsePackage()
        await futureInfoStore.setSuccessful(false)
        await futureInfoStore.callCallbackIfSet()
        await futureInfoStore.setFinished(true)
    }
    
    func getUniqueIdentifier() async -> FutureUniqueIdentifier  {
        return await futureInfoStore.futureUniqueIdentifier
    }
    
    func wasExecutedSuccessfully() async -> Bool {
        let successful = await futureInfoStore.successful
        let finished = await futureInfoStore.finished
        
        return successful && finished
    }
}
