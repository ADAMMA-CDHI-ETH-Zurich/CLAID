import Foundation

public actor ModuleFactory {
    private var registeredModuleClasses: [String: Module.Type]

    public init() {
        self.registeredModuleClasses = [:]
    }

    /// Registers a module class by its type
    public func registerModule(_ moduleType: Module.Type) throws {
        let className = String(describing: moduleType)
        try registerModule(moduleName: className, moduleType)
    }
    
    public func registerModule(moduleName: String, _ moduleType: Module.Type) throws {
        
        if registeredModuleClasses[moduleName] != nil {
            throw CLAIDError("Module class '\(moduleName)' is already registered.")
        }
        
        registeredModuleClasses[moduleName] = moduleType
        print("Registered class \(moduleName)")
    }

    /// Creates an instance of the requested module, sets its ID and type
    public func getInstance(className: String, moduleId: String) async -> Module?  {
        guard let moduleType = registeredModuleClasses[className] else {
            print("Module class \(className) not found.")
            return nil
        }
        do {
            let module = moduleType.init() // Assumes `Module` has a required initializer
            await module.setId(moduleId)
            await module.setType(className)
            return module
        } catch {
            print("Error creating instance of \(className): \(error)")
            return nil
        }
    }

    /// Checks if a module class is registered
    public func isModuleClassRegistered(_ moduleClass: String) -> Bool {
        return registeredModuleClasses.keys.contains(moduleClass)
    }

    /// Returns a list of all registered module class names
    public func getRegisteredModuleClasses() -> [String] {
        return Array(registeredModuleClasses.keys)
    }

}
