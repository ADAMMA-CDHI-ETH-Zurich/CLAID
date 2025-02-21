import Foundation



protocol Module {
    var moduleHandle: ModuleHandle { get }
    func initialize() async
    init()
}

extension Module {
    

    /// Registers a periodic function and stores it in the dictionary
    func registerPeriodicFunction(name: String, interval: TimeInterval, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addPeriodicTask(interval: interval, function: function)
        moduleHandle.tasks[name] = task
    }

    /// Registers a scheduled function and stores it in the dictionary
    func registerScheduledFunction(name: String, after delay: TimeInterval, function: @escaping @Sendable () async -> Void) async {
        let task = await moduleHandle.dispatcher.addScheduledTask(delay: delay, function: function)
        moduleHandle.tasks[name] = task
    }

    /// Cancels and removes a task using its name
    func unregisterFunction(name: String) async {
        if let task = moduleHandle.tasks[name] {
            task.cancel()
            moduleHandle.tasks.removeValue(forKey: name)
            print("Task '\(name)' unregistered.")
        } else {
            print("No task found with name '\(name)'")
        }
    }

    /// Cancels all tasks and clears the dictionary
    func cancelAllTasks() async {
        for (name, task) in moduleHandle.tasks {
            task.cancel()
            print("Task '\(name)' cancelled.")
        }
        moduleHandle.tasks.removeAll()
        print("All tasks cancelled.")
    }
    
    func setId(_ id: String) async {
        moduleHandle.id = id
    }
    
    func setType(_ type: String) async {
        moduleHandle.type = type
    }
}
