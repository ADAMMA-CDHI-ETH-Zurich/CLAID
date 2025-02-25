//
//  RemoteFunction.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

actor RemoteFunction<Return: Sendable, each Parameters: Sendable> {
    //private var futuresHandler: FutureHandler?
    //private var toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation!
    //private var remoteFunctionIdentifier: Claidservice_RemoteFunctionIdentifier
    //private var mutatorHelpers: [AbstractMutator]
    private var valid: Bool
        
    private let returnTypeExample: Return?
    private let parameterTypeExamples: (repeat each Parameters)
    
   /* init() {
        self.valid = false
        self.futuresHandler = nil
        self.toMiddlewareQueue = nil
        self.remoteFunctionIdentifier = Claidservice_RemoteFunctionIdentifier()
        self.mutatorHelpers = []
        self.returnTypeExample = nil
        self.parameterTypeExamples = nil
    }*/
    
    init(//futuresHandler: FutureHandler,
         //toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation,
         //remoteFunctionIdentifier: Claidservice_RemoteFunctionIdentifier,
        // mutatorHelpers: [AbstractMutator],
         returnTypeExample: Return,
         _ parameterTypeExamples: repeat each Parameters) {
        //self.futuresHandler = futuresHandler
        //self.toMiddlewareQueue = toMiddlewareQueue
        //self.remoteFunctionIdentifier = remoteFunctionIdentifier
        //self.mutatorHelpers = mutatorHelpers
        self.returnTypeExample = returnTypeExample
        self.parameterTypeExamples = (repeat each parameterTypeExamples)
        self.valid = true
    }
    
    /*
    static func invalidRemoteFunction() -> RemoteFunction<Return, repeat each Parameters> {
        return RemoteFunction<Return, repeat each Parameters>()
    }*/
    
    
    func getNumParams() throws -> Int {
        var count = 0
        
        
           /* for _ in repeat each parameterTypeExamples{
                count += 1
            }
            
            return count */
        return 0
    }
    
    /*func execute(_ params: repeat each Parameters) async throws -> Future<Return>? {
        guard valid else {
            throw CLAIDError("Failed to execute RemoteFunction. Function is not valid.")
        }
        
        let paramCount = try getNumParams()
        guard paramCount == mutatorHelpers.count else {
            throw CLAIDError("Number of parameters do not match. Expected \(mutatorHelpers.count), but got \(paramCount)")
        }
        
        guard let futuresHandler = self.futuresHandler else {
            throw CLAIDError("Cannot execute RemoteFunction. FuturesHandler is null.")
        }
        
        guard let returnTypeExample = self.returnTypeExample else {
            throw CLAIDError("Cannot execute RemoteFunction. The dataTypeExample is null.")
        }
        
        let future = await futuresHandler.registerNewFuture(dataTypeExample: returnTypeExample)
        var dataPackage = Claidservice_DataPackage()
        var controlPackage = Claidservice_ControlPackage()
        controlPackage.ctrlType = .ctrlRemoteFunctionRequest
        controlPackage.runtime = .swift
        
        await makeRemoteFunctionRequest(future.getUniqueIdentifier().toString(), params: repeat each params)
        
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
    
    func callAsFunction(_ params: repeat each Parameters) async throws -> Future<Return>? {
        return try await self.execute(repeat each params)
    }
    

    private func makeRemoteFunctionRequest(_ futureIdentifier: String, params: repeat each Parameters) {
        var request = Claidservice_RemoteFunctionRequest()
        
        request.remoteFunctionIdentifier = remoteFunctionIdentifier
        request.remoteFutureIdentifier = futureIdentifier
        
        var index = 0
        
        for param in repeat each params {
            var stubPackage = Claidservice_DataPackage()
            stubPackage = mutatorHelpers[index].setPackagePayloadFromObject(stubPackage, object: param)
            request.parameterPayloads.append(stubPackage.payload)
            index += 1
        }
    }*/
}
