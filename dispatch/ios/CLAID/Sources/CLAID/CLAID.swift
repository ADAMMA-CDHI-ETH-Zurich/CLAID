// The Swift Programming Language
// https://docs.swift.org/swift-book
import CLAIDNative
import Foundation

public actor CLAID {
    
    private var handle: UnsafeMutableRawPointer? = nil
    private var moduleDispatcher: ModuleDispatcher? = nil
    

    public init() {
        self.handle = nil
        self.moduleDispatcher = nil
    }

    public func start(configFile: String, hostID: String, userID: String, deviceID: String) async throws {
        
        Logger.logInfo("Starting CLAID")
        let socketPath = "unix://tmp/claid_local.grpc"
        
        handle = startCore(socketPath: socketPath, configFile: configFile, hostID: hostID, userID: userID, deviceID: deviceID)
        
        if(handle == nil) {
            throw NSError(domain: "InitError", code: 1, userInfo: [NSLocalizedDescriptionKey: "Failed to start CLAID middleware, handle is null."])
        }
        
        try await attach_swift_runtime(socketPath: socketPath)

    }
    
    func attach_swift_runtime(socketPath: String) async throws {
        
        try await self.moduleDispatcher = ModuleDispatcher(socketPath: socketPath)

        let result = try await self.moduleDispatcher?.getModuleList(registeredModuleClasses: ["None"])
    }
}
