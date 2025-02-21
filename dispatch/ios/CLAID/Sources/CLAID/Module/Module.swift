import Foundation

public class Module {
    private var id: String = ""
    private var isInitializing = false
    private var isInitialized = false
    private var isTerminating = false
    private var subscriberPublisher: Any?
    private var timers: [String: ScheduledRunnable] = [:]
    private var runnableDispatcher: RunnableDispatcher = RunnableDispatcher()

    
    init() {
        
    }
  
    func moduleFatal(_ error: String) {
        let errorMsg: String = "Module \(self.id) : \(error)"
        Logger.log(.fatal, errorMsg, logMessageEntityType: .module, logMessageEntity: self.id)
        fatalError(errorMsg)
    }
    
    func moduleError(_ error: String) {
        let errorMsg = "Module \(id ?? "unknown") : \(error)"
        Logger.log(.error, errorMsg, logMessageEntityType: .module, logMessageEntity: self.id)
    }
    
    func moduleWarning(_ warning: String) {
        let warningMsg = "Module \(id ?? "unknown") : \(warning)"
        Logger.log(.warning, warningMsg, logMessageEntityType: .module, logMessageEntity: self.id)
    }
    
    func moduleInfo(_ info: String) {
        let infoMsg = "Module \(id ?? "unknown") : \(info)"
        Logger.log(.info, infoMsg, logMessageEntityType: .module, logMessageEntity: self.id)
    }
    
    func moduleDebug(_ dbg: String) {
        let dbgMessage = "Module \(id ?? "unknown") : \(dbg)"
        Logger.log(.debugVerbose, dbgMessage, logMessageEntityType: .module, logMessageEntity: self.id)
    }
    
    func start(subscriberPublisher: Any, properties: [String: Any], mainThreadRunnablesQueue: DispatchQueue) async throws -> Bool {
        if isInitialized {
            moduleError("Initialize called twice!")
            return false
        }
        
        Logger.logInfo("Module start called")
        self.subscriberPublisher = subscriberPublisher
        self.runnableDispatcher = RunnableDispatcher()
        

        
        isInitializing = true
        isInitialized = false
        
        await initializeInternal(properties: properties)
        
        /*
        let functionRunnable = FunctionRunnable(schedule: ScheduleOnce(executionTime: Date.now)) {await
            self.initializeInternal(properties: properties)
        }
        
        try await runnableDispatcher.addRunnable(runnable: functionRunnable)*/
        
        while !isInitialized {
            sleep(1)
        }
        
        isInitializing = false
        self.subscriberPublisher = nil
        return true
    }
    
    private func initializeInternal(properties: [String: Any]) {
        Logger.logInfo("Initialize internal called")
        initialize(properties: properties)
        isInitialized = true
    }
    
    func initialize(properties: [String: Any]) {
        fatalError("Subclasses must override initialize method")
    }
    
    func setId(_ moduleId: String) {
        self.id = moduleId
    }
    
    func getId() -> String? {
        return id
    }
    
    func shutdown() async throws{
        isTerminating = true
        
        /*let functionRunnable = FunctionRunnable(schedule: ScheduleOnce(executionTime: Date.now)) {
            self.terminateInternal()
        }

        try await runnableDispatcher.addRunnable(runnable: functionRunnable)*/
        
        await terminateInternal()
        
        while isTerminating {
            sleep(1)
        }
        Logger.logInfo("Runnable dispatcher stopping")
        await runnableDispatcher.stop()
        isInitialized = false
    }
    
    private func terminateInternal() async {
        Logger.logInfo("Unregistering all periodic functions")
        await unregisterAllPeriodicFunctions()
        Logger.logInfo("Calling terminate")
        terminate()
        Logger.logInfo("Terminated")
        isTerminating = false
    }
    
    func terminate() {
        fatalError("Subclasses must override terminate method")
    }
    
    func unregisterAllPeriodicFunctions() async {
        for (_, entry) in timers {
            await entry.invalidate()
        }
        timers.removeAll()
    }
}
