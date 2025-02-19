import Foundation

actor RunnableDispatcher {
    let serialQueue = DispatchQueue(label: "module.dispatcher")
    
    func addRunnable(runnable: ScheduledRunnable) async throws {
        let date = await runnable.schedule.getExecutionTime()
        let timeInterval = date.timeIntervalSinceNow

        serialQueue.asyncAfter(deadline: .now() + timeInterval) {
            // Execute the runnable task
            runnable.run()
            
            Task {
                if(await runnable.schedule.doesRunnableHaveToBeRepeated()) {
                    try await runnable.schedule.updateExecutionTime()
                    try await self.addRunnable(runnable: runnable)
                }
            }
            

        }
    
    }
}

/*
actor RunnableDispatcher {
    
    func addRunnable(runnable: ScheduledRunnable) async {
        Task { // Launch in a new concurrent task
            let date = await runnable.schedule.getExecutionTime()
            let timeInterval = date.timeIntervalSinceNow
            
            if timeInterval > 0 {
                do {
                    try await Task.sleep(microseconds: UInt64(timeInterval * 1_000_000_000))
                } catch {
                    print("Task sleep interrupted: \(error)")
                    return
                }
            }
            
            // Execute the runnable task
            await runnable.run()
            
            // Check if the task should be repeated
            if await runnable.schedule.doesRunnableHaveToBeRepeated() {
                do {
                    try await runnable.schedule.updateExecutionTime()
                    await addRunnable(runnable: runnable) // Recursively reschedule
                } catch {
                    print("Failed to reschedule runnable: \(error)")
                }
            }
        }
    }
}
*/
