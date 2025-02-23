// The Swift Programming Language
// https://docs.swift.org/swift-book
import CLAIDNative
import Foundation

public actor CLAID {
    
    private var handle: UnsafeMutableRawPointer? = nil
    private var moduleDispatcher: ModuleDispatcher? = nil
    private var moduleManager: ModuleManager? = nil

    public init() {
        self.handle = nil
        self.moduleDispatcher = nil
    }

    public func start(configFile: String, hostID: String, userID: String, deviceID: String, moduleFactory: ModuleFactory) async throws {
        
        Logger.logInfo("Starting CLAID")
        let socketPath = "unix://tmp/claid_local.grpc"
        
        handle = startCore(socketPath: socketPath, configFile: configFile, hostID: hostID, userID: userID, deviceID: deviceID)
        
        if(handle == nil) {
            throw NSError(domain: "InitError", code: 1, userInfo: [NSLocalizedDescriptionKey: "Failed to start CLAID middleware, handle is null."])
        }
        
        try await attach_swift_runtime(socketPath: socketPath, moduleFactory: moduleFactory)

    }
    
    func attach_swift_runtime(socketPath: String, moduleFactory: ModuleFactory) async throws {
        
        
        try await self.moduleDispatcher = ModuleDispatcher(socketPath: socketPath)
        
        guard let dispatcher = self.moduleDispatcher else {
            throw CLAIDError("Failed to create CLAID ModuleDispatcher in Swift runtime.")
        }
        
        self.moduleManager = ModuleManager(
            dispatcher: dispatcher,
            moduleFactory: moduleFactory
        )
        
        guard let moduleManager = self.moduleManager else {
            throw CLAIDError("Failed to create CLAID ModuleManager in swift runtime.")
        }
        
        try await moduleManager.start()
        Logger.logInfo("CLAID has started")
    }
}
