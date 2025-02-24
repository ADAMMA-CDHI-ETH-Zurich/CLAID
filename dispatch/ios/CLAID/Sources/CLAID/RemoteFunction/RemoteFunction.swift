//
//  RemoteFunction.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//


class RemoteFunction<T> {
    private var futuresHandler: FutureHandler?
    private var toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation!
    private var remoteFunctionIdentifier: Claidservice_RemoteFunctionIdentifier
    private var mutatorHelpers: [AbstractMutator]
    private var valid: Bool
    
    private let dataTypeExample: T?
    
    init() {
        self.valid = false
        self.futuresHandler = nil
        self.toMiddlewareQueue = nil
        self.remoteFunctionIdentifier = Claidservice_RemoteFunctionIdentifier()
        self.mutatorHelpers = []
        self.dataTypeExample = nil
    }
    
    init(futuresHandler: FutureHandler, 
         toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation,
         remoteFunctionIdentifier: Claidservice_RemoteFunctionIdentifier,
         mutatorHelpers: [AbstractMutator],
         dataTypeExample: T
    ) {
        
        self.futuresHandler = futuresHandler
        self.toMiddlewareQueue = toMiddlewareQueue
        self.remoteFunctionIdentifier = remoteFunctionIdentifier
        self.mutatorHelpers = mutatorHelpers
        self.dataTypeExample = dataTypeExample
        self.valid = true
    }
    
    static func invalidRemoteFunction() -> RemoteFunction {
        return RemoteFunction()
    }
    
    func execute<Params>(_ params: Params...) async throws -> Future<T>? {
        guard valid else {
            throw CLAIDError("Failed to execute RemoteFunction. Function is not valid.")
            return nil
        }
        
        guard params.count == mutatorHelpers.count else {
            throw CLAIDError("Number of parameters do not match. Expected \(mutatorHelpers.count), but got \(params.count)")
            return nil
        }
        
        guard let futuresHandler = self.futuresHandler else {
            throw CLAIDError("Cannot execute RemoteFunction. FuturesHandler is null.")
        }
        
        guard let dataTypeExample = self.dataTypeExample else {
            throw CLAIDError("Cannot execute RemoteFunction. The dataTypeExample is null.")
        }
        
        let future = await futuresHandler.registerNewFuture(dataTypeExample: dataTypeExample)
        var dataPackage = Claidservice_DataPackage()
        var controlPackage = Claidservice_ControlPackage()
        controlPackage.ctrlType = .ctrlRemoteFunctionRequest
        controlPackage.runtime = .swift
        
        let remoteFunctionRequest = await makeRemoteFunctionRequest(future.getUniqueIdentifier().toString(), params)
        
        switch remoteFunctionIdentifier.functionType {
            case .moduleID(let moduleID):
                dataPackage.targetModule = remoteFunctionIdentifier.moduleID
            default:
                break // Do nothing for other cases
        }

        guard let toMiddlewareQueue = self.toMiddlewareQueue else {
            throw CLAIDError("Cannot execute RemoteFunction. Queue toMiddlewareQueue is null.")
        }
        toMiddlewareQueue.yield(dataPackage)
        return future
    }
    
    func callAsFunction<Params>(_ params: Params...) async throws -> Future<T>? {
        return try await self.execute(params)
    }
    
    private func makeRemoteFunctionRequest<Params>(_ futureIdentifier: String, _ params: [Params]) {
        var request = Claidservice_RemoteFunctionRequest()

        request.remoteFunctionIdentifier = remoteFunctionIdentifier
        request.remoteFutureIdentifier = futureIdentifier
        
        for (index, param) in params.enumerated() {
            var stubPackage = Claidservice_DataPackage()
            stubPackage = mutatorHelpers[index].setPackagePayloadFromObject(stubPackage, object: param)
            request.parameterPayloads.append(stubPackage.payload)
        }
    }
}
