

class ModuleManager {
    
    let moduleDispatcher: ModuleDispatcher
    let moduleFactory: ModuleFactory
    
    func instantiate_module(module_id: String, module_class: String) {
            
    }
    
    public init(moduleDispatcher: ModuleDispatcher, moduleFactory: ModuleFactory) {
        self.moduleDispatcher = moduleDispatcher
        self.moduleFactory = moduleFactory
    }
    
    public func start() {
        
    }
}
