//
//  Future.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//


public actor Future<T: Sendable>: AbstractFuture {
  
    // Required by AbstractFuture
    public let futureInfoStore: FutureInfoStore
    public let futureTable: FuturesTable
    
    // Whether the typed callback has been set
    private var typedCallbackSet = false
    
    // The typed callback function
    private var callbackFunction: (@Sendable (T) -> Void)?
    
    private let dataTypeExample: T
    private let mutator: Mutator<T>

    private func decodePayload(_ dataPackage: Claidservice_DataPackage) -> T {
        let data = self.mutator.getPackagePayload(dataPackage)
        return data
    }
    
    
    public init(futuresTable: FuturesTable, uniqueIdentifier: FutureUniqueIdentifier, dataTypeExample: T) {
        self.futureTable = futuresTable
        self.futureInfoStore = FutureInfoStore(futureUniqueIdentifier: uniqueIdentifier)
        self.dataTypeExample = dataTypeExample
        self.mutator = TypeMapping.getMutator(type(of: dataTypeExample))
    }
    
    
    public func callAsFunction() async -> T? {
        // calls out to your existing `awaitResponse()`
        let responsePackage = await self.awaitResponse()

        guard let package = responsePackage, await self.wasExecutedSuccessfully() else {
            return nil
        }

        return decodePayload(package)
    }
    
    public func callAsFunction(timeout: Duration) async throws -> T? {
        let responsePackage = try await self.awaitResponse(timeoutDuration: timeout)
        
        guard let package = responsePackage, await self.wasExecutedSuccessfully() else {
            return nil
        }
        
        return decodePayload(package)
    }
    
    
    /// Register a typed callback that receives the decoded `T` once the future completes.
    public func then(_ callback: @Sendable @escaping (T) -> Void) async {
        self.callbackFunction = callback
        self.typedCallbackSet = true
        
        let typedCallbackSet = true
        
        // We bind our typed callback to the untyped callback used internally
        await self.thenUntyped { [self] (dataPackage: Claidservice_DataPackage?) in
            let decoded = await self.decodePayload(dataPackage!)
            await self.callbackFunction?(decoded)
        }
    }
}
