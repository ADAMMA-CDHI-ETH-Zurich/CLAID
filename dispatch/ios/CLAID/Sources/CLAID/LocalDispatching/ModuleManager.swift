import Foundation

@MainActor
class ModuleManager {
    private let dispatcher: ModuleDispatcher
    private let moduleFactory: ModuleFactory
    
    private var remoteFunctionHandler: RemoteFunctionHandler?
    private var remoteFunctionRunnableHandler: RemoteFunctionRunnableHandler?
    private var runningModules: [String: CLAIDModule] = [:]
    private var subscriberPublisher: ChannelSubscriberPublisher?
    private var running = false

  //  private var restartControlPackage: DataPackage?

    init(dispatcher: ModuleDispatcher, moduleFactory: ModuleFactory) {
        self.dispatcher = dispatcher
        self.moduleFactory = moduleFactory
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
        

        //runningModules[moduleId] = module
        return true
    }

    func instantiateModules(moduleList: Claidservice_ModuleListResponse) async -> Bool {
        for descriptor in moduleList.descriptors {
            
            if self.runningModules.contains(where: {descriptor.moduleID == $0.key}){
                print("Skipping Module as it is already loaded \(descriptor.moduleID)")
                continue
            }
            
            if !(await instantiateModule(moduleId: descriptor.moduleID, moduleClass: descriptor.moduleClass)) {
                print("Failed to instantiate Module \"\(descriptor.moduleID)\" (class: \"\(descriptor.moduleClass)\").\nThe Module class was not registered to the ModuleFactory.")
                return false
            }
        }
        return true
    }

    func initializeModules(moduleList: Claidservice_ModuleListResponse, subscriberPublisher: ChannelSubscriberPublisher) async throws -> Bool {
        
        guard let remoteFunctionHandler = self.remoteFunctionHandler else {
            throw CLAIDError("Cannot initialize modules, RemoteFunctionHandler is null.")
        }
        
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

    func getTemplatePackagesOfModules() async throws -> [String: [Claidservice_DataPackage]] {
        var moduleChannels: [String: [Claidservice_DataPackage]] = [:]
        for moduleId in runningModules.keys {
            
            guard let subPub = subscriberPublisher else {
                throw CLAIDError("Cannot get template packages of Channel, subscriberPublisher is null.")
            }
            
            let templatePackagesForModule = await subPub.getChannelTemplatePackagesForModule(moduleId: moduleId) 
            moduleChannels[moduleId] = templatePackagesForModule
        }
        return moduleChannels
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
        Logger.logInfo("Received package")
        if dataPackage.hasControlVal {
            Logger.logInfo("ModuleManager received DataPackage with controlVal \(dataPackage.controlVal.ctrlType) (\(dataPackage.controlVal.ctrlType))")
            await handlePackageWithControlVal(package: dataPackage)
            return
        }
        
        let channelName = dataPackage.channel
        let moduleId = dataPackage.targetModule
                
        guard let subPub = subscriberPublisher else {
            Logger.logFatal("Failed to process received data package. ChannelSubscriberPublisher is null.")
            return
        }
        
        if await !subPub.isDataPackageCompatibleWithChannel(dataPackage: dataPackage, receiverModule: moduleId) {
            let payloadCaseName = dataPackage.payload.messageType
            let expectedPayloadCaseName = await subPub.getPayloadCaseOfChannel(channelName: channelName, receiverModule: moduleId)
            Logger.logFatal("ModuleManager received package with target for Module \(moduleId) on Channel \(channelName), " +
                            "however the data type of payload of the package did not match the data type of the Channel. " +
                            "Expected payload type \(expectedPayloadCaseName) but got \(payloadCaseName).")
            return
        }
        
        let subscriberList = await subPub.getSubscriberInstancesOfModule(channelName: channelName, moduleId: moduleId)
        
        for subscriber in subscriberList {
            Logger.logInfo("Invoking subscriber \(subscriber)")
            await subscriber.onNewData(dataPackage: dataPackage)
        }
    }

    func handlePackageWithControlVal(package: Claidservice_DataPackage) async {
        switch package.controlVal.ctrlType {
        case .ctrlConnectedToRemoteServer:
            for (_, module) in runningModules {
                await module.notifyConnectedToRemoteServer()
            }
        case .ctrlDisconnectedFromRemoteServer:
            for (_, module) in runningModules {
                await module.notifyDisconnectedFromRemoteServer()
            }
        case .ctrlUnloadModules:
            Logger.logFatal("Control message CTR_UNLOAD_MODULES not yet implemented.")
            /*Logger.logInfo("ModuleManager received ControlPackage with code CTRL_UNLOAD_MODULES")
            Logger.logInfo("Unloading modules!")
            shutdownModules()
            let response = DataPackage()
            response.controlVal.ctrlType = .ctrlUnloadModulesDone
            response.controlVal.runtime = .runtimeCpp
            response.sourceHost = package.targetHost
            response.targetHost = package.sourceHost
            toModuleDispatcherQueue.append(response)
            Logger.logInfo("Unloading modules done and acknowledgement will be sent out")*/
        case .ctrlRestartRuntime:
            Logger.logFatal("Control message CTRL_RESTART_RUNTIME not yet implemented.")

            /*if let restartThread = restartThread {
                restartThread.join()
            }
            restartThread = Thread {
                self.restart()
            }
            restartThread?.start()
            restartControlPackage = package*/
        case .ctrlPauseModule:
            Logger.logFatal("Control message CTRL_PAUSE_MODULE not yet implemented.")
            /*if let module = runningModules[package.targetModule] {
                module.pauseModule()
            }*/
        case .ctrlUnpauseModule:
            Logger.logFatal("Control message CTRL_UNPAUSE_MODULE not yet implemented.")

            /*if let module = runningModules[package.targetModule] {
                module.resumeModule()
            }*/
        case .ctrlAdjustPowerProfile:
            Logger.logFatal("Control message CTR_UNLOAD_MODULES not yet implemented.")

            /*if let module = runningModules[package.targetModule] {
                module.adjustPowerProfile(powerProfile: package.controlVal.powerProfile)
            }*/
        case .ctrlRemoteFunctionRequest:
            await handleRemoteFunctionRequest(package)
        case .ctrlRemoteFunctionResponse:
            await handleRemoteFunctionResponse(package)
        default:
            break
        }
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

    func getModuleById(moduleId: String) async -> CLAIDModule? {
        return runningModules[moduleId]
    }
    
    func handleRemoteFunctionRequest(_ remoteFunctionRequest: Claidservice_DataPackage) async {
        let request = remoteFunctionRequest.controlVal.remoteFunctionRequest
        
        switch request.remoteFunctionIdentifier.functionType {
        case .runtime:
            await handleRuntimeRemoteFunctionExecution(remoteFunctionRequest)
        case .moduleID:
            await handleModuleRemoteFunctionExecution(remoteFunctionRequest)
        case .none:
            Logger.logError("Received invalid remoteFunctionRequest \(remoteFunctionRequest). RemoteFunctionType is None.")
        }
        
    }
    
    func handleRuntimeRemoteFunctionExecution(_ request: Claidservice_DataPackage) async {
        
        guard let remoteFunctionRunnableHandler = remoteFunctionRunnableHandler else {
            Logger.logError("Cannot handle remot function request! RemoteFunctionRunnableHandler is null.")
            return 
        }
        
        let result = await remoteFunctionRunnableHandler.executeRemoteFunctionRunnable(request)
        
        if !result {
            Logger.logError("Swift runtime failed to execute RPC request")
            return
        }
    }
    
    func handleModuleRemoteFunctionExecution(_ request: Claidservice_DataPackage) async {
        let remoteFunctionRequest = request.controlVal.remoteFunctionRequest
        let moduleId = remoteFunctionRequest.remoteFunctionIdentifier.moduleID
        
        guard let module = runningModules[moduleId] else {
            Logger.logError("Failed to execute remote function request. Could not find Module \"\(moduleId)\"")
            return
        }
        
        await module.executeRPCRequest(request)
    }
    
    func handleRemoteFunctionResponse(_ remoteFunctionResponse: Claidservice_DataPackage) async {
        guard let remoteFunctionHandler = self.remoteFunctionHandler else {
            Logger.logError("Cannot handle remoteFunctionRespose, remoteFunctionHandler of Module is null.")
            return
        }
        
        await remoteFunctionHandler.handleResponse(remoteFunctionResponse)
    }
    

    func getRemoteFunctionHandler() -> RemoteFunctionHandler? {
        return remoteFunctionHandler
    }
    
    func start() async throws -> Bool {
        
        self.remoteFunctionHandler = RemoteFunctionHandler(toMiddlewareQueue: await dispatcher.getToMiddlewareContinuation())
        self.remoteFunctionRunnableHandler = RemoteFunctionRunnableHandler(
            entityName: "RUNTIME_SWIFT",
            toMiddlewareQueue: await dispatcher.getToMiddlewareContinuation()
        )
        
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

        let examplePackagesOfModules = try await getTemplatePackagesOfModules()
        try await dispatcher.initRuntime(channelExamplePackages: examplePackagesOfModules)
        
        running = true

        Task {
            await self.readFromMiddleware()
        }
        
        // This should block indefinitely?
        Task {
            try await dispatcher.sendReceivePackages()
        }

        return true
    }
    
    public func addPreloadedModule(moduleId: String, module: CLAIDModule) async {
        print("Adding preloaded module \(moduleId)")
        self.runningModules[moduleId] = module
    }
}
