import Foundation

public final class ModuleFactory : @unchecked Sendable{
    // internal storage
    private var registeredModuleClasses: [String: CLAIDModule.Type] = [:]
    private let lock = NSLock()

    public init() { }

    /// Registers a module class by its type
    public func registerModule(_ moduleType: CLAIDModule.Type) throws {
        let className = String(describing: moduleType)
        try registerModule(moduleName: className, moduleType)
    }
    
    /// Registers a module class under an explicit name
    public func registerModule(moduleName: String, _ moduleType: CLAIDModule.Type) throws {
        lock.lock()
        defer { lock.unlock() }

        if registeredModuleClasses[moduleName] != nil {
            throw CLAIDError("Module class '\(moduleName)' is already registered.")
        }
        
        registeredModuleClasses[moduleName] = moduleType
        print("Registered class \(moduleName)")
    }

    /// Creates an instance of the requested module, sets its ID and type
    public func getInstance(className: String, moduleId: String) -> CLAIDModule? {
        // lock only around dictionary access
        lock.lock()
        let moduleType = registeredModuleClasses[className]
        lock.unlock()
        
        guard let type = moduleType else {
            print("Module class \(className) not found.")
            return nil
        }

        // instantiate & configure outside the lock
        let module = type.init(id: moduleId)      // assume synchronous init

        return module
    }

    /// Checks if a module class is registered
    public func isModuleClassRegistered(_ moduleClass: String) -> Bool {
        lock.lock()
        defer { lock.unlock() }
        return registeredModuleClasses.keys.contains(moduleClass)
    }

    /// Returns a list of all registered module class names
    public func getRegisteredModuleClasses() -> [String] {
        lock.lock()
        defer { lock.unlock() }
        return Array(registeredModuleClasses.keys)
    }
}
