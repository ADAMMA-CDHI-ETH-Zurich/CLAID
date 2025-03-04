//
//  RemoteFunctionRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

public final class RemoteFunctionRunnable<ReturnType: Sendable, each Parameter: Sendable> : AbstractRemoteFunctionRunnable {
    private let functionName: String
    private let function: @Sendable (repeat each Parameter) -> ReturnType
    private let mutatorHelpers: [AbstractMutator]
    private let params: (repeat each Parameter)
    
    init(functionName: String,
         returnTypeExample: ReturnType,
         paramExamples: repeat each Parameter,
         function: @escaping @Sendable (repeat each Parameter) -> ReturnType) {
        
        self.functionName = functionName
        self.function = function
         
        var helpers: [AbstractMutator] = []
        repeat helpers.append(TypeMapping.getMutator(type(of: each paramExamples)))
        self.mutatorHelpers = helpers
        
        self.params = (repeat each paramExamples)
    }
    
    func executeRemoteFunctionRequest(_ rpcRequest: Claidservice_DataPackage) -> Claidservice_DataPackage? {
        let executionRequest = rpcRequest.controlVal.remoteFunctionRequest
        let remoteFunctionIdentifier = executionRequest.remoteFunctionIdentifier
        
        let parameterPayloads = executionRequest.parameterPayloads
        if parameterPayloads.count != mutatorHelpers.count {
            print("Failed to execute RemoteFunctionRunnable \(functionName). Number of parameters do not match.")
            return nil
        }
     
        var count = 0
        let parameter = (repeat unpack(each params, &count, parameterPayloads))

        
        let result: ReturnType = function(repeat each parameter)
        let remoteFunctionResult = RemoteFunctionRunnableResult.makeSuccessfulResult(result)
        return makeRPCResponsePackage(result: remoteFunctionResult, rpcRequest: rpcRequest)
    }
    
    // Helper function to bridge runtime data to static types
    private func unpackParameters(payloads: [Claidservice_Blob], _ par: (repeat each Parameter)) -> (repeat each Parameter) {
        // This is the tricky part - we need to map runtime data to the compile-time Parameter pack
        
       // let xx =  (repeat unpack(each params))
        
        /*var index = 0
        for var param in repeat each self.params {
            var stubPackage = Claidservice_DataPackage()
            stubPackage.payload = payloads[index]
            let helper = mutatorHelpers[index]
            param = repeat helper.getPackagePayloadAsObject(stubPackage) as! each Parameter
            index += 1
        }
        
        return ()
        */
        // This is a conceptual placeholder - actual casting to [(repeat each Parameter)] is complex
        return params
    }
    
    func unpack<T>(_ paramExample: T, _ index: inout Int, _ payloads: [Claidservice_Blob]) -> T {
        var stubPackage = Claidservice_DataPackage()
        
        stubPackage.payload = payloads[index]
        let helper = mutatorHelpers[index]
        let val = helper.getPackagePayloadAsObject(stubPackage) as! T
        index += 1
        return val
    }
    
    private func makeRPCResponsePackage(
            result: RemoteFunctionRunnableResult<ReturnType>,
            rpcRequest: Claidservice_DataPackage
        ) -> Claidservice_DataPackage? {
            var responsePackage = Claidservice_DataPackage()
            
            responsePackage.sourceModule = rpcRequest.targetModule
            responsePackage.targetModule = rpcRequest.sourceModule
            
            var ctrlPackage = responsePackage.controlVal
            ctrlPackage.ctrlType = .ctrlRemoteFunctionResponse
            ctrlPackage.runtime = rpcRequest.controlVal.runtime
            
            let executionRequest = rpcRequest.controlVal.remoteFunctionRequest
            ctrlPackage.remoteFunctionReturn = makeRemoteFunctionReturn(result: result, executionRequest: executionRequest)
            responsePackage.controlVal = ctrlPackage
            setReturnPackagePayload(package: &responsePackage, result: result)
            
            return responsePackage
        }
        
        private func makeRemoteFunctionReturn(
            result: RemoteFunctionRunnableResult<ReturnType>,
            executionRequest: Claidservice_RemoteFunctionRequest
        ) -> Claidservice_RemoteFunctionReturn {
            var remoteFunctionReturn = Claidservice_RemoteFunctionReturn()
            
            remoteFunctionReturn.executionStatus = result.getStatus()
            remoteFunctionReturn.remoteFunctionIdentifier = executionRequest.remoteFunctionIdentifier
            remoteFunctionReturn.remoteFutureIdentifier = executionRequest.remoteFutureIdentifier
            
            return remoteFunctionReturn
        }
        
        private func setReturnPackagePayload(
            package: inout Claidservice_DataPackage,
            result: RemoteFunctionRunnableResult<ReturnType>
        ) {
            if let returnValue = result.getReturnValue(), ReturnType.self != Void.self {
                let mutator = TypeMapping.getMutator(ReturnType.self)
                package = mutator.setPackagePayload(packet: package, value: returnValue)
            }
        }
}
