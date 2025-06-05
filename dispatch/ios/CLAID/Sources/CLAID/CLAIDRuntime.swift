//
//  CLAIDLoader.swift
//  iOSCLAIDTestApp
//
//  Created by Patrick Langer on 29.05.2025.
//

@testable import Spezi
import CLAID
import Foundation

public final class CLAIDRuntime : ServiceModule, @unchecked Sendable {
 

    @Application(\.spezi) private var spezi

    
    private let host: String
    private let user: String
    private let device: String
    private let claid = CLAID()
    
    private var resolvedConfigPath: String? = nil
    
    public init(
        id: String = "CLAIDRuntime",
        host: String = "defaultHost",
        user: String = "defaultUser",
        device: String = "defaultDevice"
    ) {
        self.host = host
        self.user = user
        self.device = device
    }
    
    /*
    public func configure() {
        // Lock here if needed to protect shared state
        
        func getTestConfigPath() -> String? {
            // Get the path to the resource inside the Swift package
            if let fileURL = Bundle.main.url(forResource: "test_config", withExtension: "json") {
                return fileURL.path // Convert URL to a file path string
            }
            print("Test config not found!!")
            
            return nil
        }
        if let testConfigPath = getTestConfigPath() {
            Task {
                try await start(
                    configFile: testConfigPath,
                    hostID: "test_host",
                    userID: "test_user",
                    deviceID: "test_device"
                )
            }
        }
    }*/
    
    @MainActor
    public func run() async {
        let config = await makeCLAIDConfig()
        print("Config \(config)")
        do {
            try writeOutCLAIDConfig(config: config)
        } catch {
            // TODO: Add exception handling
            print("Exception: \(error)")
            return
        }
        
        let modules = await spezi.modules
        guard let configPath = resolvedConfigPath else {
            // TODO: Throw exception
            return
        }

        do {
            for module in modules {
                if let claidModule = module as? CLAIDModule {
                    let moduleId = await claidModule.getId()
                    try await self.claid.registerModule(type(of: claidModule))
                    self.claid.addPreloadedModule(moduleId: moduleId, module: claidModule)
                }
            }
            print("Starting CLAID 123 \(self.host)")
            try await self.claid.start(
                configFile: configPath,
                hostID: self.host,
                userID: self.user,
                deviceID: self.device
            )
        } catch {
            print("Error starting CLAID: \(error)")
        }
    }

    
    func getConfigFromModule(module: CLAIDModule) -> Claidservice_ModuleConfig {
        var moduleConfig = Claidservice_ModuleConfig()
        moduleConfig.inputChannels = module.getInputChannels()
        moduleConfig.outputChannels = module.getOutputChannels()
        moduleConfig.id = module.getId()
        moduleConfig.type = module.getType()
        
        return moduleConfig
    }
    
    @MainActor
    func getAllModuleConfigs() -> [Claidservice_ModuleConfig]{
        print("Get all module configs \(spezi.modules)")
        let claidModules = spezi.modules.compactMap {
            $0 as? any CLAIDModule
        }
        print("Get all module configs \(claidModules)")

        var moduleConfigs: [Claidservice_ModuleConfig] = []
        
        for module in claidModules {
            let config = getConfigFromModule(module: module)
            moduleConfigs.append(config)
        }
        
        return moduleConfigs
    }
    
    @MainActor
    func makeCLAIDConfig() -> Claidservice_CLAIDConfig {
        var config = Claidservice_CLAIDConfig()
        var hostconfig = Claidservice_HostConfig()
        hostconfig.hostname = self.host
        hostconfig.modules = getAllModuleConfigs()
        
        config.hosts = [hostconfig]
        
        return config
    }
    
    func writeOutCLAIDConfig(
        config: Claidservice_CLAIDConfig
    ) throws {
        let tempDirectory = FileManager.default.temporaryDirectory
        
        let claidConfigOutputPath = "CLAIDConfig.json"
        
        let tempOutputPath = tempDirectory.appendingPathComponent(claidConfigOutputPath).path
        self.resolvedConfigPath = tempOutputPath
        
        // Serialize back to JSON
        let jsonData = try config.jsonUTF8Data()
        
        // Save to output path
        try jsonData.write(to: URL(fileURLWithPath: tempOutputPath))
    }
}
