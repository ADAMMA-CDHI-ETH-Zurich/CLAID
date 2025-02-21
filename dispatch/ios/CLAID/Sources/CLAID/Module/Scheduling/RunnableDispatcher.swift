import Foundation

actor RunnableDispatcher {
    private let serialQueue = DispatchQueue(label: "module.dispatcher")
    private var scheduledTasks: [Date: DispatchWorkItem] = [:] // Keyed by Date
    private var isRunning: Bool = false

    func start() {
        isRunning = true
    }

    func stop() {
        isRunning = false
        for (_, task) in scheduledTasks {
            task.cancel()
        }
        scheduledTasks.removeAll()
    }

    func addRunnable(runnable: ScheduledRunnable) async throws {
        guard isRunning else { return }
        let date = await runnable.schedule.getExecutionTime()
        let timeInterval = max(date.timeIntervalSinceNow, 0) // Don't schedule past tasks

        let workItem = DispatchWorkItem { [weak self] in
            Task { [weak self] in
                guard let self = self else { return }
                guard await self.isRunning else { return }
                
                await runnable.run()
                
                if await runnable.schedule.doesRunnableHaveToBeRepeated() {
                    try await runnable.schedule.updateExecutionTime()
                    try await self.addRunnable(runnable: runnable)
                }
                
                await self.removeTask(for: date)
            }
        }

        scheduledTasks[date] = workItem
        serialQueue.asyncAfter(deadline: .now() + timeInterval, execute: workItem)
    }

    func removeRunnable(for date: Date) {
        if let task = scheduledTasks[date] {
            task.cancel()
            scheduledTasks.removeValue(forKey: date)
        }
    }

    private func removeTask(for date: Date) {
        scheduledTasks.removeValue(forKey: date)
    }
}
