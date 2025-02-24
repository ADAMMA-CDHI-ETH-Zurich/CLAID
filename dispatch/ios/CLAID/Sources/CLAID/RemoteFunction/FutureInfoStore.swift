//
//  FutureInfo.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

public actor FutureInfoStore : Sendable {
    var finished = false
    var successful = false
    var failed = false
    
    var responsePackage: Claidservice_DataPackage? = nil
    
    var callback: ((Claidservice_DataPackage?) async -> Void)?
    var callbackSet: Bool = false
    
    var futureUniqueIdentifier: FutureUniqueIdentifier
    
    public init(futureUniqueIdentifier: FutureUniqueIdentifier) {
        self.futureUniqueIdentifier = futureUniqueIdentifier
    }
    
    public func setCallback(callback: @escaping @Sendable ((Claidservice_DataPackage?) async -> Void)) {
        self.callback = callback
        self.callbackSet = true
    }
    
    public func callCallbackIfSet() async {
        if callbackSet {
            await callback?(self.responsePackage)
        }
    }
    
    public func setFinished(_ finished: Bool) {
        self.finished = finished
    }
    
    public func setSuccessful(_ successful: Bool) {
        self.successful = successful
    }
    
    public func setFailed(_ failed: Bool) {
        self.failed = false
    }
    
    public func setResponsePackage(_ responsePackage: Claidservice_DataPackage) {
        self.responsePackage = responsePackage
    }
    
    public func unsetResponsePackage() {
        self.responsePackage = nil
    }
}
