//
//  RemoteFunctionStatus.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//


struct RemoteFunctionRunnableResult<T> {
    private let returnValue: T?
    private let status: Claidservice_RemoteFunctionStatus
    
    init(returnValue: T?, status: Claidservice_RemoteFunctionStatus) {
        self.returnValue = returnValue
        self.status = status
    }
    
    static func makeSuccessfulResult(_ returnValue: T) -> RemoteFunctionRunnableResult<T> {
        return RemoteFunctionRunnableResult(returnValue: returnValue, status: .statusOk)
    }
    
    static func makeFailedResult(status: Claidservice_RemoteFunctionStatus) -> RemoteFunctionRunnableResult<T> {
        return RemoteFunctionRunnableResult(returnValue: nil, status: status)
    }
    
    func getStatus() -> Claidservice_RemoteFunctionStatus {
        return self.status
    }
    
    func getReturnValue() -> T? {
        return self.returnValue
    }
}
