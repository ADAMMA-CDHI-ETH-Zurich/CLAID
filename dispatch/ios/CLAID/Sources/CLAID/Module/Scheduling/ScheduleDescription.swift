import Foundation

public protocol ScheduleDescription : Sendable {
    func doesRunnableHaveToBeRepeated() async -> Bool
    func updateExecutionTime() async throws
    func getExecutionTime() async -> Date
}
