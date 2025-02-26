

public actor ModuleHandle {
    public let dispatcher = RunnableDispatcher()
    public var tasks: [String: Task<Void, Never>] = [:]
    public var counter: Int = 0  // A variable to be modified by tasks
    public var id: String = ""
    public var type: String = ""
    public var subscriberPublisher: ChannelSubscriberPublisher?
    public var remoteFunctionHandler: RemoteFunctionHandler?
    public var remoteFunctionRunnableHandler: RemoteFunctionRunnableHandler?
    public var properties: Properties?
    public var initialized: Bool = false
    
    public init() {

    }

    public func addTask(_ name: String, _ task: Task<Void, Never>) {
        tasks[name] = task
    }
    
    public func removeTask(_ name: String) {
        tasks.removeValue(forKey: name)
    }
    
    public func removeAllTasks() {
        tasks.removeAll()
    }
    
    public func setInitialized(_ initialized: Bool) {
        self.initialized = initialized
    }
    
    public func isInitialized() -> Bool {
        return initialized
    }
    
    public func setId(_ id: String) {
        self.id = id
    }
    
    public func getId() -> String {
        return id
    }
    
    public func setType(_ type: String) {
        self.type = type
    }
    
    public func getType() -> String {
        return type
    }
    
    public func setSubscribePublisher(_ subscriberPublisher: ChannelSubscriberPublisher) {
        self.subscriberPublisher = subscriberPublisher
    }
    
    public func setRemoteFunctionHandler(_ remoteFunctionHandler: RemoteFunctionHandler) {
        self.remoteFunctionHandler = remoteFunctionHandler
    }
    
    public func setRemoteFunctionRunnableHandler(_ remoteFunctionRunnableHandler: RemoteFunctionRunnableHandler) {
        self.remoteFunctionRunnableHandler = remoteFunctionRunnableHandler
    }
    
    public func setProperties(_ properties: Properties) {
        self.properties = properties
    }
}
