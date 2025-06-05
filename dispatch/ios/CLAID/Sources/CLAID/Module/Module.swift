import Foundation
import protocol Spezi.Module

public protocol CLAIDModule : Sendable, Spezi.Module {
    var moduleHandle: ModuleHandle { get }
    
    init(id: String)
    
    func loadPropertiesFromConfig(properties: Properties) async
    func run() async throws
    func terminate() async
    func notifyConnectedToRemoteServer() async
    func notifyDisconnectedFromRemoteServer() async
    func publish<T: Sendable>(_ channelName: String, dataTypeExample: T) async throws -> Channel<T>
    func subscribe<T: Sendable>(_ channelName: String, dataTypeExample: T, callback: @escaping @Sendable (ChannelData<T>) async -> Void) async throws -> Channel<T>
    func registerPeriodicFunction(name: String, interval: Duration, function: @escaping @Sendable () async -> Void) async
    func registerScheduledFunction(name: String, after delay: Duration, function: @escaping @Sendable () async -> Void) async
    func unregisterFunction(name: String) async
    func configure()
    
    func inputChannels(_ channels: [String:String]) -> CLAIDModule
    func outputChannels(_ channels: [String:String]) -> CLAIDModule
}

extension CLAIDModule {
    
    public static func makeDefaultHandle(id: String) -> ModuleHandle {
        let className = String(describing: Self.self)
        return ModuleHandle(
            id ?? className,
            className
        )
    }

    @MainActor
    public func configure() {
        let className = String(describing: Self.self)
        print("\(className) configure")
    }
    
    /// Registers a periodic function and stores it in the dictionary
    public func registerPeriodicFunction(name: String, interval: Duration, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addPeriodicTask(interval: interval.timeInterval, function: function)
        await moduleHandle.addTask(name, task)
    }

    /// Registers a scheduled function and stores it in the dictionary
    public func registerScheduledFunction(name: String, after delay: Duration, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addScheduledTask(delay: delay.timeInterval, function: function)
        await moduleHandle.addTask(name, task)
    }

    /// Cancels and removes a task using its name
    public func unregisterFunction(name: String) async {
        if let task = await moduleHandle.tasks[name] {
            task.cancel()
            await moduleHandle.removeTask(name)
            print("Task '\(name)' unregistered.")
        } else {
            print("No task found with name '\(name)'")
        }
    }
    
    private func assertCanPublish(_ channelName: String) async throws  {
        if(await !moduleHandle.isInitialized()) {
            throw CLAIDError("Cannot publish channel \(channelName). Publishing is only allowed during initialization (i.e., the first call of the initialize function).")
        }
    }
        
    public func publish<T: Sendable>(_ channelName: String, dataTypeExample: T) async throws -> Channel<T> {
        
        try await assertCanPublish(channelName)
        
        guard let subscriberPublisher = await moduleHandle.subscriberPublisher else {
            throw CLAIDError("Failed to publish Channel \"\(channelName)\". Module has not started yet, please only subscribe or publish Channels in the initialize function.")
        }
        
        return await subscriberPublisher.publish(
            dataTypeExample: dataTypeExample,
            moduleId: await self.moduleHandle.getId(),
            channelName: channelName
        )
    }
    
    private func assertCanSubscribe(_ channelName: String) async throws {
        if(await !moduleHandle.isInitialized()) {
            throw CLAIDError("Cannot subscribe channel \(channelName). Subscribing is only allowed during initialization (i.e., the first call of the initialize function).")
        }
    }
        
    public func subscribe<T: Sendable>(_ channelName: String, dataTypeExample: T, callback: @escaping @Sendable (ChannelData<T>) async -> Void) async throws -> Channel<T> {
            try await assertCanSubscribe(channelName)
            
            guard let subscriberPublisher = await moduleHandle.subscriberPublisher else {
                throw CLAIDError("Failed to subscribe to Channel \"\(channelName)\". Module has not started yet, please only subscribe or publish Channels in the initialize function.")
            }
            
            let subscriber = Subscriber(
                dataTypeExample: dataTypeExample,
                callback: callback,
                callbackDispatcher: moduleHandle.dispatcher
            )
            return await subscriberPublisher.subscribe(
                dataTypeExample: dataTypeExample,
                moduleId:  await self.moduleHandle.getId(),
                channelName: channelName,
                subscriber: subscriber
            )
    }
    
    public func mapRemoteFunctionOfModule<Return: Sendable, each Parameters: Sendable>(
        moduleId: String,
        functionName: String,
        returnType: Return,
        parameterTypes: repeat each Parameters
    ) async throws -> RemoteFunction<Return, repeat each Parameters>  {
            
            
        let id = await self.moduleHandle.getId()
        if moduleId == id {
            await moduleFatal("Cannot map remote function. Module tried to map function \"\(functionName)\" of itself, which is not allowed.")
            //return RemoteFunction<Return, repeat each Parameters>.invalidRemoteFunction()
        }
        guard let remoteFunctionHandler = await self.moduleHandle.remoteFunctionHandler else {
            throw CLAIDError("Cannot map remote function of module, remoteFunctionHandler is null.")
        }
                
        let futuresHandler = await remoteFunctionHandler.futuresHandler
        let toMiddlewareQueue = await remoteFunctionHandler.toMiddlewareQueue
        let identifier = await remoteFunctionHandler.makeRemoteFunctionIdentifier(moduleId: moduleId, functionName: functionName)
        
        let remoteFunction = RemoteFunction(
            futuresHandler: futuresHandler,
            toMiddlewareQueue: toMiddlewareQueue!,
            remoteFunctionIdentifier: identifier,
            returnTypeExample: returnType,
            parameterTypeExamples: repeat each parameterTypes)
        
        
        return remoteFunction
    }

    public func mapRemoteFunctionOfRuntime<Return, each Parameters>(
        runtime: Claidservice_Runtime,
        functionName: String,
        returnTypeExample: Return,
        parameterTypeExamples: repeat each Parameters
    ) async throws -> RemoteFunction<Return, repeat each Parameters> {
        guard let remoteFunctionHandler = await self.moduleHandle.remoteFunctionHandler else {
            throw CLAIDError("Cannot map remote function of runtime, remoteFunctionHandler is null.")
        }
    
        let futuresHandler = await remoteFunctionHandler.futuresHandler
        let toMiddlewareQueue = await remoteFunctionHandler.toMiddlewareQueue
        let identifier = await remoteFunctionHandler.makeRemoteFunctionIdentifier(
            runtime: runtime,
            functionName: functionName
        )
        
        let remoteFunction = RemoteFunction(
            futuresHandler: futuresHandler,
            toMiddlewareQueue: toMiddlewareQueue!,
            remoteFunctionIdentifier: identifier,
            returnTypeExample: returnTypeExample,
            parameterTypeExamples: repeat each parameterTypeExamples
        )
        
        return remoteFunction
    }

    func executeRPCRequest(_ rpcRequest: Claidservice_DataPackage) async {
        let id = await self.moduleHandle.id
        if rpcRequest.targetModule != id {
            await moduleError("Failed to execute RPC request. RPC is targeted for Module \"\(rpcRequest.sourceModule)\", but we are Module \"\(id)\".")
            return
        }
        
        guard let remoteFunctionRunnableHandler = await self.moduleHandle.remoteFunctionRunnableHandler else {
            await moduleError("Failed to execute rpcRequest. The remoteFunctionRunnableHandler of the Module is null.")
            return
        }
        
        let result = await remoteFunctionRunnableHandler.executeRemoteFunctionRunnable(rpcRequest)
        if !result {
            await moduleError("Failed to execute rpcRequest")
            return
        }
    }

    func registerRemoteFunction<ReturnType: Sendable, each Parameter: Sendable>(
        functionName: String,
        returnTypeExample: ReturnType,
        _ paramExamples: repeat each Parameter,
        function: @escaping @Sendable (repeat each Parameter) -> ReturnType
    ) async throws {
    
        
        guard let remoteFunctionRunnableHandler = await self.moduleHandle.remoteFunctionRunnableHandler else {
            throw CLAIDError("Failed to register remote function \"\(functionName)\". The remoteFunctionRunnableHandler of the Module is null.")
        }
        
        let runnable = RemoteFunctionRunnable<ReturnType, repeat each Parameter>(
            functionName: functionName,
            returnTypeExample: returnTypeExample,
            paramExamples: repeat each paramExamples,
            function: function
        )
        
        await remoteFunctionRunnableHandler.registerRunnable(functionName: functionName, runnable: runnable)
    }
    
    /// Cancels all tasks and clears the dictionary
    func cancelAllTasks() async {
        for (name, task) in await moduleHandle.tasks {
            task.cancel()
            print("Task '\(name)' cancelled.")
        }
        await moduleHandle.removeAllTasks()
        print("All tasks cancelled.")
    }
    
    public func moduleFatal(_ error: String) async {
        let moduleId = await moduleHandle.getId()
        let errorMsg = "Module \(moduleId): \(error)"
        Logger.log(.fatal, errorMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
        
    public func moduleError(_ error: String) async {
        let moduleId = await moduleHandle.getId()
        let errorMsg = "Module \(moduleId): \(error)"
        Logger.log(.error, errorMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    public func moduleWarning(_ warning: String) async {
        let moduleId = await moduleHandle.getId()
        let warningMsg = "Module \(moduleId): \(warning)"
        Logger.log(.warning, warningMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    public func moduleInfo(_ info: String) async {
        let moduleId = await moduleHandle.getId()
        let infoMsg = "Module \(moduleId): \(info)"
        Logger.log(.info, infoMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    public func moduleDebug(_ debug: String) async {
        let moduleId = await moduleHandle.getId()
        let dbgMsg = "Module \(moduleId): \(debug)"
        Logger.log(.debugVerbose, dbgMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    public func setId(_ id: String) async {
        await self.moduleHandle.setId(id)
    }
    
    public func getId() -> String {
        return moduleHandle.getId()
    }
    
    
    public func getType() -> String {
        return moduleHandle.getType()
    }
    
    public func start(
        subscriberPublisher: ChannelSubscriberPublisher,
        remoteFunctionHandler: RemoteFunctionHandler,
        properties: Properties
    ) async throws {
        
        if await !moduleHandle.isValid() {
            throw CLAIDError.init("ModuleHandle is invalid. This might happen if the same Module type is registered multiple times.")
        }
        
        await moduleHandle.setSubscribePublisher(subscriberPublisher)
        await moduleHandle.setRemoteFunctionHandler(remoteFunctionHandler)
        await moduleHandle.setProperties(properties)
        
        let queue = subscriberPublisher.toModuleManagerQueue
        let remoteFunctionRunnableHandler = RemoteFunctionRunnableHandler(entityName: await getId(), toMiddlewareQueue: queue)
        await moduleHandle.setRemoteFunctionRunnableHandler(remoteFunctionRunnableHandler)
        
        await moduleHandle.setInitialized(true)
        await loadPropertiesFromConfig(properties: properties)
        try await run()//initialize(properties: properties)
    }
    
    public func shutdown() async {
        await terminate()
    }
        
    public func notifyConnectedToRemoteServer() async {
        
    }
    
    public func notifyDisconnectedFromRemoteServer() async {
        
    }
    
    public func loadPropertiesFromConfig(properties: Properties) async {
        
    }
    
    public func getInputChannels() -> [String:String] {
        return moduleHandle.getInputChannels()
    }

    public func getOutputChannels() -> [String:String] {
        return moduleHandle.getOutputChannels()
    }
    
    public func inputChannels(_ channels: [String:String]) -> CLAIDModule {
        moduleHandle.setInputChannels(channels: channels)
        return self
    }
    
    public func outputChannels(_ channels: [String:String]) -> CLAIDModule {
        moduleHandle.setOutputChannels(channels: channels)
        return self
    }
}
