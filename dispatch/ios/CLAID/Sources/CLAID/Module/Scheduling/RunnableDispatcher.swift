import Foundation

public actor RunnableDispatcher {
    /// Registers a periodic function that executes repeatedly at a specified interval
    func addPeriodicTask(interval: TimeInterval, function: @escaping @Sendable () async -> Void) -> Task<Void, Never> {
        let task = Task {
            while !Task.isCancelled {
                await function()
                try? await Task.sleep(nanoseconds: UInt64(interval * 1_000_000_000))
            }
        }
        return task
    }

    /// Registers a scheduled function that executes once after a specified delay
    func addScheduledTask(delay: TimeInterval, function: @escaping @Sendable () async -> Void) -> Task<Void, Never> {
        let task = Task {
            try? await Task.sleep(nanoseconds: UInt64(delay * 1_000_000_000))
            await function()
        }
        return task
    }
}
