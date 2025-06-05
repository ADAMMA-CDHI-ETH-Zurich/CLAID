// The Swift Programming Language
// https://docs.swift.org/swift-book
import CLAIDNative
import Foundation
import protocol Spezi.Module

public class CLAID: Spezi.Module {
    
    private var handle: UnsafeMutableRawPointer? = nil
    private var c_runtime_handle: UnsafeMutableRawPointer? = nil
    private var moduleDispatcher: ModuleDispatcher? = nil
    private var moduleManager: ModuleManager? = nil
    private var moduleFactory = ModuleFactory()
    private var preloadedModules: [String: CLAIDModule] = [:]
    
    private let lock = NSLock()

    public init() {}


    public func start(configFile: String, hostID: String, userID: String, deviceID: String) async throws {
        
        Logger.logInfo("Starting CLAID")
        let socketPath = "localhost:1337"

        guard let documentsPathUri = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first else {
            throw CLAIDError("Failed to get documents directory URI.")
        }

        let documentsPath = documentsPathUri.path

        let startedHandle = startCoreWithEventTracker(
            socketPath: socketPath,
            configFile: configFile,
            hostID: hostID,
            userID: userID,
            deviceID: deviceID,
            commonDataPath: documentsPath
        )

        guard let validHandle = startedHandle else {
            throw CLAIDError("Failed to start CLAID middleware, handle is null.")
        }

        let runtimeHandle = try attachCppRuntime(handle: validHandle)

        guard let validRuntimeHandle = runtimeHandle else {
            throw CLAIDError("Failed to start CLAID C++ runtime, C++ runtime handle is null.")
        }

        self.handle = validHandle
        self.c_runtime_handle = validRuntimeHandle

        try await attach_swift_runtime(socketPath: socketPath)
    }

    private func attach_swift_runtime(socketPath: String) async throws {
        let dispatcher = try await ModuleDispatcher(socketPath: socketPath)
        let manager = await ModuleManager(dispatcher: dispatcher, moduleFactory: moduleFactory)

        var modulesToLoad: [String: CLAIDModule] = [:]

        self.moduleDispatcher = dispatcher
        self.moduleManager = manager
        modulesToLoad = self.preloadedModules

        for (moduleId, module) in modulesToLoad {
            await manager.addPreloadedModule(moduleId: moduleId, module: module)
        }

        try await manager.start()
        Logger.logInfo("CLAID has started")
    }

    public func getRemoteFunctionHandler() async -> RemoteFunctionHandler? {
        var manager: ModuleManager?

        manager = self.moduleManager

        return await manager?.getRemoteFunctionHandler()
    }

    public func registerModule(_ moduleType: CLAIDModule.Type) async throws {
        // Assuming moduleFactory is thread-safe
        try await moduleFactory.registerModule(moduleType)
    }

    public func addPreloadedModule(moduleId: String, module: CLAIDModule) {
        lock.lock()
        preloadedModules[moduleId] = module
        lock.unlock()
    }
}
