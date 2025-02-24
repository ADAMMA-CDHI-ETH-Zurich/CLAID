//
//  RemoteFunctionRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

class RemoteFunctionRunnable<ReturnType, each Parameter> : AbstractRemoteFunctionRunnable {
    private let functionName: String
    private let function: ([(repeat each Parameter)]) -> ReturnType
    private var mutatorHelpers: [AbstractMutator]
    
    init(functionName: String,
         paramExamples: repeat each Parameter,
         function: @escaping ([(repeat each Parameter)]) -> ReturnType) {
        
        self.functionName = functionName
        self.function = function
        self.mutatorHelpers = []
        
        repeat self.mutatorHelpers.append(TypeMapping.getMutator(type(of: each paramExamples)))
    }
    
    func executeRemoteFunctionRequest(_ rpcRequest: Claidservice_DataPackage) -> Claidservice_DataPackage? {
        let executionRequest = rpcRequest.controlVal.remoteFunctionRequest
        let remoteFunctionIdentifier = executionRequest.remoteFunctionIdentifier
        
        let parameterPayloads = executionRequest.parameterPayloads
        if parameterPayloads.count != mutatorHelpers.count {
            print("Failed to execute RemoteFunctionRunnable \(functionName). Number of parameters do not match.")
            return nil
        }
        
        // We'll use a helper function to unpack and cast parameters
        guard let parameters = unpackParameters(payloads: parameterPayloads) else {
            print("Failed to unpack parameters for \(functionName)")
            return nil
        }
        
        let result: ReturnType = function(parameters)
        let remoteFunctionResult = RemoteFunctionRunnableResult.makeSuccessfulResult(result)
        return makeRPCResponsePackage(result: remoteFunctionResult, rpcRequest: rpcRequest)
    }
    
    // Helper function to bridge runtime data to static types
    private func unpackParameters(payloads: [Claidservice_Blob]) -> [(repeat each Parameter)]? {
        // This is the tricky part - we need to map runtime data to the compile-time Parameter pack
        var parameters: [Any] = []
        
        for (index, helper) in mutatorHelpers.enumerated() {
            var stubPackage = Claidservice_DataPackage()
            stubPackage.payload = payloads[index]
            let param = helper.getPackagePayloadAsObject(stubPackage)
            parameters.append(param)
        }
        
        // This is a conceptual placeholder - actual casting to [(repeat each Parameter)] is complex
        return parameters as? [(repeat each Parameter)]
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
