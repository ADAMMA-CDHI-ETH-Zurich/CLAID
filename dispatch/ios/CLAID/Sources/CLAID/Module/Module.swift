import Foundation

public class Module {
    private var id: String = ""
    private var isInitializing = false
    private var isInitialized = false
    private var isTerminating = false
    private var subscriberPublisher: Any?
    private var timers: [String: ScheduledRunnable] = [:]
    private var runnableDispatcher: RunnableDispatcher?

    
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
    
    func start(subscriberPublisher: Any, properties: [String: Any], mainThreadRunnablesQueue: DispatchQueue) -> Bool {
        if isInitialized {
            moduleError("Initialize called twice!")
            return false
        }
        
        Logger.logInfo("Module start called")
        self.subscriberPublisher = subscriberPublisher
        self.runnableDispatcher = RunnableDispatcher(queue: mainThreadRunnablesQueue)
        
        guard let dispatcher = runnableDispatcher, dispatcher.start() else {
            moduleError("Failed to start RunnableDispatcher.")
            return false
        }
        
        isInitializing = true
        isInitialized = false
        
        let functionRunnable = FunctionRunnableWithParams(initializeInternal)
        functionRunnable.setParams(properties)
        
        dispatcher.addRunnable(
            ScheduledRunnable(
                runnable: functionRunnable,
                schedule: ScheduleOnce(Date())
            )
        )
        
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
    
    func shutdown() {
        isTerminating = true
        let functionRunnable = FunctionRunnable(terminateInternal)
        runnableDispatcher?.addRunnable(
            ScheduledRunnable(
                runnable: functionRunnable,
                schedule: ScheduleOnce(Date())
            )
        )
        while isTerminating {
            sleep(1)
        }
        Logger.logInfo("Runnable dispatcher stopping")
        runnableDispatcher?.stop()
        isInitialized = false
    }
    
    private func terminateInternal() {
        Logger.logInfo("Unregistering all periodic functions")
        unregisterAllPeriodicFunctions()
        Logger.logInfo("Calling terminate")
        terminate()
        Logger.logInfo("Terminated")
        isTerminating = false
    }
    
    func terminate() {
        fatalError("Subclasses must override terminate method")
    }
    
    func unregisterAllPeriodicFunctions() {
        for (_, entry) in timers {
            entry.runnable.invalidate()
        }
        timers.removeAll()
    }
}
