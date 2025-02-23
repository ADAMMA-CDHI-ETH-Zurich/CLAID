import Foundation

actor ModuleManager {
    private let dispatcher: ModuleDispatcher
    private let moduleFactory: ModuleFactory
    private let remoteFunctionHandler = RemoteFunctionHandler()
    private let remoteFunctionRunnableHandler = RemoteFunctionRunnableHandler()
    

    private var runningModules: [String: Module] = [:]
    private var subscriberPublisher: ChannelSubscriberPublisher?
    private var running = false
  //  private var restartControlPackage: DataPackage?

    init(dispatcher: ModuleDispatcher, moduleFactory: ModuleFactory) {
        self.dispatcher = dispatcher
        self.moduleFactory = moduleFactory
     // self.remoteFunctionHandler = RemoteFunctionHandler(fromModulesChannel: dispatcher.getFromModulesChannel())
     // self.remoteFunctionRunnableHandler = RemoteFunctionRunnableHandler(runtime: "RUNTIME_SWIFT", fromModulesChannel: dispatcher.getFromModulesChannel())
        
    }

    func instantiateModule(moduleId: String, moduleClass: String) async -> Bool {
        if !(await moduleFactory.isModuleClassRegistered(moduleClass)) {
            print("ModuleManager: Failed to instantiate Module of class \"\(moduleClass)\" (id: \(moduleId))\nA Module with this class is not registered to the ModuleFactory.")
            return false
        }

        print("Loaded Module with id \"\(moduleId)\" (class: \"\(moduleClass)\").")
       /* guard let module = await moduleFactory.getInstance(className: moduleClass, moduleId: moduleId) else {
            print("Failed to instantiate Module with id \"\(moduleId)\" (class: \"\(moduleClass)\").")
            return false
        }*/
        let module = TestModule()
        await module.setId("Test")

        runningModules[moduleId] = module
        return true
    }

    func instantiateModules(moduleList: Claidservice_ModuleListResponse) async -> Bool {
        for descriptor in moduleList.descriptors {
            if !(await instantiateModule(moduleId: descriptor.moduleID, moduleClass: descriptor.moduleClass)) {
                print("Failed to instantiate Module \"\(descriptor.moduleID)\" (class: \"\(descriptor.moduleClass)\").\nThe Module class was not registered to the ModuleFactory.")
                return false
            }
        }
        return true
    }

    func initializeModules(moduleList: Claidservice_ModuleListResponse, subscriberPublisher: ChannelSubscriberPublisher) async throws -> Bool {
        for descriptor in moduleList.descriptors {
            guard let module = runningModules[descriptor.moduleID] else {
                print("Failed to initialize Module \"\(descriptor.moduleID)\" (class: \"\(descriptor.moduleClass)\").\nThe Module was not loaded.")
                return false
            }

            print("Calling module.start() for Module \"\(await module.getId())\".")
            let properties = Properties(properties: descriptor.properties)
            try await module.start(
                subscriberPublisher: subscriberPublisher,
                remoteFunctionHandler: remoteFunctionHandler,
                properties: properties
            )
            print("Module \"\(await module.getId())\" has started.")
        }
        return true
    }

    func getTemplatePackagesOfModules() async -> [String: [Claidservice_DataPackage]] {
        var moduleChannels: [String: [Claidservice_DataPackage]] = [:]
        for moduleId in runningModules.keys {
            let templatePackagesForModule = await subscriberPublisher?.getChannelTemplatePackagesForModule(moduleId: moduleId) ?? []
            moduleChannels[moduleId] = templatePackagesForModule
        }
        return moduleChannels
    }

    func start() async throws -> Bool {
        let registeredModuleClasses = await moduleFactory.getRegisteredModuleClasses()



        let moduleList = try await dispatcher.getModuleList(registeredModuleClasses: registeredModuleClasses)
        print("Received ModuleListResponse: \(moduleList)")

        if !(await instantiateModules(moduleList: moduleList)) {
            print("ModuleDispatcher: Failed to instantiate Modules.")
            return false
        }

        if subscriberPublisher == nil {
            subscriberPublisher = await ChannelSubscriberPublisher(toModuleManagerQueue: dispatcher.getToMiddlewareContinuation())
        }

        if !(try await initializeModules(moduleList: moduleList, subscriberPublisher: subscriberPublisher!)) {
            print("Failed to initialize Modules.")
            return false
        }

        let examplePackagesOfModules = await getTemplatePackagesOfModules()
        try await dispatcher.initRuntime(channelExamplePackages: examplePackagesOfModules)
        
        // This should block indefinitely?
        try await dispatcher.sendReceivePackages()
        running = true

        Task {
            await self.readFromMiddleware()
        }

        return true
    }

    func shutdownModules() async {
        for module in runningModules.values {
            await module.shutdown()
        }
        runningModules.removeAll()
    }

    func stop() async {
        await shutdownModules()
        running = false
    }

    func readFromMiddleware() async {
        while running {
            let stream = await dispatcher.getFromMiddlewareStream()
            for try await package in stream {
                await onDataPackageReceivedFromMiddleware(dataPackage: package)
            }
        }
    }

    func onDataPackageReceivedFromMiddleware(dataPackage: Claidservice_DataPackage) async {
       
    }

    /*func restart() async {
        await stop()
        print("Shutting down dispatcher")
        await dispatcher.shutdown()
        print("Dispatcher was shut down")

        while !(await dispatcher.wasInputStreamCancelled()) {
            try? await Task.sleep(nanoseconds: 50_000_000)
        }

        _ = await start()

        var responseBuilder = DataPackage()
        var ctrlPackageBuilder = ControlPackage()
        ctrlPackageBuilder.ctrlType = .CTRL_RESTART_RUNTIME_DONE
        ctrlPackageBuilder.runtime = .RUNTIME_SWIFT
        responseBuilder.controlVal = ctrlPackageBuilder
        await dispatcher.getFromModulesChannel().add(responseBuilder)
    }

    func postLogMessage(logMessage: LogMessage) async {
        guard running else { return }
        var responseBuilder = DataPackage()
        var ctrlPackageBuilder = ControlPackage()
        ctrlPackageBuilder.ctrlType = .CTRL_LOCAL_LOG_MESSAGE
        ctrlPackageBuilder.runtime = .RUNTIME_SWIFT
        ctrlPackageBuilder.logMessage = logMessage
        responseBuilder.controlVal = ctrlPackageBuilder
    }*/

    func getModuleById(moduleId: String) async -> Module? {
        return runningModules[moduleId]
    }

    func getRemoteFunctionHandler() -> RemoteFunctionHandler {
        return remoteFunctionHandler
    }
}
