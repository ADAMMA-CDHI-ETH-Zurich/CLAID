import Foundation



public protocol Module : Sendable {
    var moduleHandle: ModuleHandle { get }
    
    init()
    
    func initialize(properties: Properties) async
    func terminate() async
}

extension Module {
    /// Registers a periodic function and stores it in the dictionary
    func registerPeriodicFunction(name: String, interval: TimeInterval, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addPeriodicTask(interval: interval, function: function)
        moduleHandle.tasks[name] = task
    }

    /// Registers a scheduled function and stores it in the dictionary
    func registerScheduledFunction(name: String, after delay: TimeInterval, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addScheduledTask(delay: delay, function: function)
        moduleHandle.tasks[name] = task
    }

    /// Cancels and removes a task using its name
    func unregisterFunction(name: String) async {
        if let task = moduleHandle.tasks[name] {
            task.cancel()
            moduleHandle.tasks.removeValue(forKey: name)
            print("Task '\(name)' unregistered.")
        } else {
            print("No task found with name '\(name)'")
        }
    }
    
    private func assertCanPublish(_ channelName: String) -> Bool {
        guard moduleHandle.isInitialized else {
                moduleError("Cannot publish channel \(channelName). Publishing is only allowed during initialization (i.e., the first call of the initialize function).")
                return false
            }
            return true
        }
        
        func publish<T>(_ channelName: String, dataType: T.Type) -> Channel<T> {
            guard assertCanPublish(channelName) else {
                return Channel.newInvalidChannel(channelName)
            }
            return subscriberPublisher.publish(self, DataType(dataType), channelName)
        }
        
        private func assertCanSubscribe(_ channelName: String) -> Bool {
            guard moduleHandle.isInitialized else {
                moduleError("Cannot subscribe channel \(channelName). Subscribing is only allowed during initialization (i.e., the first call of the initialize function).")
                return false
            }
            return true
        }
        
        func subscribe<T>(_ channelName: String, dataTypeClass: T.Type, callback: @escaping (ChannelData<T>) -> Void) -> Channel<T> {
            guard assertCanSubscribe(channelName) else {
                return Channel.newInvalidChannel(channelName)
            }
            
            let dataType = DataType(dataTypeClass)
            return subscriberPublisher.subscribe(self, dataType, channelName, Subscriber(dataType: dataType, callback: callback, runnableDispatcher: runnableDispatcher))
        }

    /// Cancels all tasks and clears the dictionary
    func cancelAllTasks() async {
        for (name, task) in moduleHandle.tasks {
            task.cancel()
            print("Task '\(name)' cancelled.")
        }
        moduleHandle.tasks.removeAll()
        print("All tasks cancelled.")
    }
    
    public func setId(_ id: String) async {
        moduleHandle.id = id
    }
    
    public func getId() async -> String {
        return moduleHandle.id
    }
    
    public func setType(_ type: String) async {
        moduleHandle.type = type
    }
    
    public func start(subscriberPublisher: ChannelSubscriberPublisher, remoteFunctionHandler: RemoteFunctionHandler, properties: Properties) async {
        moduleHandle.subscriberPublisher = subscriberPublisher
        moduleHandle.remoteFunctionHandler = remoteFunctionHandler
        moduleHandle.properties = properties
        
        await initialize(properties: properties)
        moduleHandle.isInitialized = true
    }
    
    public func shutdown() async {
        await terminate()
    }
    
    func moduleFatal(_ error: String) -> Never {
        let errorMsg = "Module \(moduleHandle.id): \(error)"
        Logger.log(.fatal, errorMsg, logMessageEntityType:.module, logMessageEntity:moduleHandle.id)
    }
        
    func moduleError(_ error: String) {
        let errorMsg = "Module \(moduleHandle.id): \(error)"
        Logger.log(.error, errorMsg, logMessageEntityType:.module, logMessageEntity:moduleHandle.id)
    }
    
    func moduleWarning(_ warning: String) {
        let warningMsg = "Module \(moduleHandle.id): \(warning)"
        Logger.log(.warning, warningMsg, logMessageEntityType:.module, logMessageEntity:moduleHandle.id)
    }
    
    func moduleInfo(_ info: String) {
        let infoMsg = "Module \(moduleHandle.id): \(info)"
        Logger.log(.info, infoMsg, logMessageEntityType:.module, logMessageEntity:moduleHandle.id)
    }
    
    func moduleDebug(_ debug: String) {
        let dbgMsg = "Module \(moduleHandle.id): \(debug)"
        Logger.log(.debugVerbose, dbgMsg, logMessageEntityType:.module, logMessageEntity:moduleHandle.id)
    }
}
