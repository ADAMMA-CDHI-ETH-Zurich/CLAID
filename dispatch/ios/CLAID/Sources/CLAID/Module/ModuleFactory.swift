import Foundation

public actor ModuleFactory {
    private var registeredModuleClasses: [String: Module.Type] = [:]

    init() {
    }

    /// Registers a module class by its type
    func registerModule(_ moduleType: Module.Type) -> Bool {
        let className = String(describing: moduleType)
        registeredModuleClasses[className] = moduleType
        print("Registered class \(className)")
        return true
    }

    /// Creates an instance of the requested module, sets its ID and type
    func getInstance(className: String, moduleId: String) async -> Module?  {
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
    func isModuleClassRegistered(_ moduleClass: String) -> Bool {
        return registeredModuleClasses.keys.contains(moduleClass)
    }

    /// Returns a list of all registered module class names
    func getRegisteredModuleClasses() -> [String] {
        return Array(registeredModuleClasses.keys)
    }

}
