// The Swift Programming Language
// https://docs.swift.org/swift-book
import CLAIDNative
import Foundation

public actor CLAID {
    private static var handle: UnsafeMutableRawPointer? = nil
    private static var c_runtime_handle: UnsafeMutableRawPointer? = nil
    private static var moduleDispatcher: ModuleDispatcher? = nil
    private static var moduleManager: ModuleManager? = nil
        
    public init() {

    }

    public static func start(configFile: String, hostID: String, userID: String, deviceID: String, moduleFactory: ModuleFactory) async throws {
        
        Logger.logInfo("Starting CLAID")
        let socketPath = "localhost:1337"
        
        guard let documentsPathUri = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first else {
            throw CLAIDError("Failed to get documents directory URI.")
        }
        
        let documentsPath = documentsPathUri.path

        handle = startCoreWithEventTracker(
            socketPath: socketPath,
            configFile: configFile,
            hostID: hostID,
            userID: userID,
            deviceID: deviceID,
            commonDataPath: documentsPath
        )
        
        if(handle == nil) {
            throw CLAIDError("Failed to start CLAID middleware, handle is null.")
        }
        
        c_runtime_handle = try attachCppRuntime(handle: handle)
        if(c_runtime_handle == nil) {
            throw CLAIDError("Failed to start CLAID C++ runtime, C++ runtime handle is null.")
        }
        
       try await attach_swift_runtime(socketPath: socketPath, moduleFactory: moduleFactory)
    }
    
    static func attach_swift_runtime(socketPath: String, moduleFactory: ModuleFactory) async throws {
        
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
    
    public static func getRemoteFunctionHandler() async -> RemoteFunctionHandler? {
        return await self.moduleManager?.getRemoteFunctionHandler()
    }
}
