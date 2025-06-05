

public class ModuleHandle : @unchecked Sendable {
    
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
    public var valid: Bool = false
    public var inputChannels: [String: String] = [:]
    public var outputChannels: [String: String] = [:]
    
    public init() {
        valid = false
    }
    
    public static func InvalidHandle() -> ModuleHandle {
        return ModuleHandle()
    }
    
    public init(_ id: String, _ type: String) {
        self.id = id
        self.type = type
        self.valid = true
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
    
    public func isValid() -> Bool {
        return valid
    }
    
    public func setInputChannels(channels: [String: String]) {
        inputChannels = channels
    }
    
    public func setOutputChannels(channels: [String: String]) {
        outputChannels = channels
    }
    
    public func getInputChannels() -> [String: String] {
        return inputChannels
    }
    
    public func getOutputChannels() -> [String: String] {
        return outputChannels
    }
}
