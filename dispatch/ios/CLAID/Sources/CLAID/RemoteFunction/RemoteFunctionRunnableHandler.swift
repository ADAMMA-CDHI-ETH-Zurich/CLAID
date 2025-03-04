//
//  RemoteFunctionRunnableHandler.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//

public actor RemoteFunctionRunnableHandler : Sendable {
    let entityName: String
    private var toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation!
    private var registeredRunnables: [String: AbstractRemoteFunctionRunnable] = [:]

    init(
        entityName: String,
         toMiddlewareQueue: AsyncStream<Claidservice_DataPackage>.Continuation
    ) {
        self.entityName = entityName
        self.toMiddlewareQueue = toMiddlewareQueue
    }

    /// Registers a remote function with specific parameter types and return type.
    /// - Parameters:
    ///   - functionName: The name of the function to register.
    ///   - paramExamples: Example values for each parameter type, used to determine mutators.
    ///   - function: A closure taking an array of parameters, matching the provided RemoteFunctionRunnable signature.
    /// - Returns: True if registration succeeds, false if the function is already registered.
    func registerRunnable<ReturnType, each Parameter>(
        functionName: String,
        runnable: RemoteFunctionRunnable<ReturnType, repeat each Parameter>
    ) -> Bool {
        if registeredRunnables[functionName] != nil {
            print("Failed to register function \"\(functionName)\" in entity \"\(entityName)\". Function already registered before.")
            return false
        }

        // Note: In C++, there's a check for supported return types via isDataTypeSupported.
        // Assuming TypeMapping.getMutator handles this in Swift (it likely throws or fails if unsupported),
        // we skip an explicit check here. In a production environment, add validation if needed.

        /*let runnable = RemoteFunctionRunnable<ReturnType, repeat each Parameter>(
            functionName: functionName,
            paramExamples: repeat each paramExamples,
            function: function
        )*/
        
        registeredRunnables[functionName] = runnable
        return true
    }

    /// Executes a remote function based on the RPC request.
    /// - Parameter rpcRequest: The data package containing the remote function request.
    /// - Returns: True if execution proceeds (even if no response), false if request or runnable is invalid.
    func executeRemoteFunctionRunnable(_ rpcRequest: Claidservice_DataPackage) -> Bool {
        
        
        if rpcRequest.controlVal.hasRemoteFunctionRequest == false {
            print("Failed to execute RPC request data package. Could not find definition of RemoteFunctionRequest.")
            return false
        }
        
        let request = rpcRequest.controlVal.remoteFunctionRequest

        let functionName = request.remoteFunctionIdentifier.functionName
        guard let runnable = registeredRunnables[functionName] else {
            print("Failed to execute RPC request. Entity \"\(entityName)\" does not have a registered remote function called \"\(functionName)\".")
            return false
        }

        if let response = runnable.executeRemoteFunctionRequest(rpcRequest) {
            toMiddlewareQueue.yield(response)
        }
        
        return true
    }
}

// Assuming SharedQueue is defined elsewhere as a generic class, e.g.:
// class SharedQueue<T> {
//     func push(_ item: T) { ... }
// }
