import Foundation



public protocol Module : Actor {
    var moduleHandle: ModuleHandle { get }
    
    init()
    
    func initialize(properties: Properties) async throws
    func terminate() async
    func notifyConnectedToRemoteServer() async
    func notifyDisconnectedFromRemoteServer() async
}

extension Module {
    /// Registers a periodic function and stores it in the dictionary
    func registerPeriodicFunction(name: String, interval: Duration, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addPeriodicTask(interval: interval.timeInterval, function: function)
        await moduleHandle.addTask(name, task)
    }

    /// Registers a scheduled function and stores it in the dictionary
    func registerScheduledFunction(name: String, after delay: Duration, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addScheduledTask(delay: delay.timeInterval, function: function)
        await moduleHandle.addTask(name, task)
    }

    /// Cancels and removes a task using its name
    func unregisterFunction(name: String) async {
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
        
    func publish<T: Sendable>(_ channelName: String, dataTypeExample: T) async throws -> Channel<T> {
        
        try await assertCanPublish(channelName)
        
        guard let subscriberPublisher = await moduleHandle.subscriberPublisher else {
            throw CLAIDError("Failed to publish Channel \"\(channelName)\". Module has not started yet, please only subscribe or publish Channels in the initialize function.")
        }
        
        return await subscriberPublisher.publish(
            dataTypeExample: dataTypeExample,
            module: self,
            channelName: channelName
        )
    }
    
    private func assertCanSubscribe(_ channelName: String) async throws {
        if(await !moduleHandle.isInitialized()) {
            throw CLAIDError("Cannot subscribe channel \(channelName). Subscribing is only allowed during initialization (i.e., the first call of the initialize function).")
        }
    }
        
    func subscribe<T: Sendable>(_ channelName: String, dataTypeExample: T, callback: @escaping @Sendable (ChannelData<T>) async -> Void) async throws -> Channel<T> {
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
                module: self,
                channelName: channelName,
                subscriber: subscriber
            )
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
    
    func moduleFatal(_ error: String) async {
        let moduleId = await moduleHandle.getId()
        let errorMsg = "Module \(moduleId): \(error)"
        Logger.log(.fatal, errorMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
        
    func moduleError(_ error: String) async {
        let moduleId = await moduleHandle.getId()
        let errorMsg = "Module \(moduleId): \(error)"
        Logger.log(.error, errorMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    func moduleWarning(_ warning: String) async {
        let moduleId = await moduleHandle.getId()
        let warningMsg = "Module \(moduleId): \(warning)"
        Logger.log(.warning, warningMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    func moduleInfo(_ info: String) async {
        let moduleId = await moduleHandle.getId()
        let infoMsg = "Module \(moduleId): \(info)"
        Logger.log(.info, infoMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    func moduleDebug(_ debug: String) async {
        let moduleId = await moduleHandle.getId()
        let dbgMsg = "Module \(moduleId): \(debug)"
        Logger.log(.debugVerbose, dbgMsg, logMessageEntityType:.module, logMessageEntity:moduleId)
    }
    
    public func setId(_ id: String) async {
        await self.moduleHandle.setId(id)
    }
    
    public func getId() async -> String {
        return await moduleHandle.getId()
    }
    
    public func setType(_ type: String) async {
        await moduleHandle.setType(type)
    }
    
    public func start(
        subscriberPublisher: ChannelSubscriberPublisher,
        remoteFunctionHandler: RemoteFunctionHandler,
        properties: Properties
    ) async throws {
        await moduleHandle.setSubscribePublisher(subscriberPublisher)
        await moduleHandle.setRemoteFunctionHandler(remoteFunctionHandler)
        await moduleHandle.setProperties(properties)
        
        await moduleHandle.setInitialized(true)
        try await initialize(properties: properties)
    }
    
    public func shutdown() async {
        await terminate()
    }
        
    public func notifyConnectedToRemoteServer() async {
        
    }
    
    public func notifyDisconnectedFromRemoteServer() async {
        
    }
}
