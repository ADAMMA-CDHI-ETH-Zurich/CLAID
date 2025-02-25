//
//  RemoteFunctionHandler.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//



public actor RemoteFunctionHandler {
    private var futuresHandler = FutureHandler()
    private var toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation!

    init(toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation) {
        self.toMiddlewareQueue = toMiddlewareQueue
    }
    
    private func makeRemoteFunctionIdentifier(
        runtime: Claidservice_Runtime,
        functionName: String
    ) -> Claidservice_RemoteFunctionIdentifier {
        var remoteFunctionIdentifier = Claidservice_RemoteFunctionIdentifier()
        remoteFunctionIdentifier.functionName = functionName
        remoteFunctionIdentifier.runtime = runtime
        return remoteFunctionIdentifier
    }
    
    private func makeRemoteFunctionIdentifier(
        moduleId: String,
        functionName: String)
    -> Claidservice_RemoteFunctionIdentifier {
        var remoteFunctionIdentifier = Claidservice_RemoteFunctionIdentifier()
        remoteFunctionIdentifier.functionName = functionName
        remoteFunctionIdentifier.moduleID = moduleId
        return remoteFunctionIdentifier
    }
    /*
    func mapRuntimeFunction<Return, each Parameter>(
        runtime: Claidservice_Runtime,
        functionName: String,
        returnDataTypeExample: Return,
        parameterTypeExamples: repeat each Parameter
    ) -> RemoteFunction<Return, repeat each Parameter> {
        
        
        return RemoteFunction(
            futuresHandler: futuresHandler,
            toMiddlewareQueue: toMiddlewareQueue,
            remoteFunctionIdentifier: makeRemoteFunctionIdentifier(runtime: runtime, functionName: functionName),
            mutatorHelpers: [],
            returnTypeExample: returnDataTypeExample,
            parameterTypeExamples: repeat each parameterTypeExamples
        )
    }
    
    func mapModuleFunction<Return, each Parameter>(
        targetModule: String,
        functionName: String,
        returnDataTypeExample: Return,
        parameterTypeExamples: repeat each Parameter
    ) -> RemoteFunction<Return, repeat each Parameter> {

        return RemoteFunction(
            futuresHandler: futuresHandler,
            toMiddlewareQueue: toMiddlewareQueue,
            remoteFunctionIdentifier: makeRemoteFunctionIdentifier(moduleId: targetModule, functionName: functionName),
            mutatorHelpers: [],
            returnTypeExample: returnDataTypeExample,
            parameterTypeExamples: repeat each parameterTypeExamples
        )
    }*/
    
    func handleResponse(_ remoteFunctionResponse: Claidservice_DataPackage) async {
        guard remoteFunctionResponse.controlVal.hasRemoteFunctionReturn else {
            print("Failed to handle remote function response. Did not find RemoteFunctionReturn data")
            return
        }
        
        let remoteFunctionReturn = remoteFunctionResponse.controlVal.remoteFunctionReturn
        let futureIdentifier = remoteFunctionReturn.remoteFutureIdentifier
        let uniqueIdentifier = FutureUniqueIdentifier.fromString(futureIdentifier)
        
        guard let future = await futuresHandler.lookupFuture(identifier: uniqueIdentifier) else {
            return
        }
        
        if remoteFunctionReturn.executionStatus != .statusOk {
            Logger.logWarning("Remote function failed. Future with identifier \(futureIdentifier) failed with status \(remoteFunctionReturn.executionStatus)")
            await future.setFailed()
            return
        }
        
        await future.setResponse(remoteFunctionResponse)
    }
}
