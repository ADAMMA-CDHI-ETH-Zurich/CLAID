import Foundation

public actor Publisher<T> {
    private let moduleId: String
    private let channelName: String
    private let mutator: Mutator<T>
    
    // Continuation for sending data to the ModuleManager
    private let toModuleManagerQueue: AsyncStream<Claidservice_DataPackage>.Continuation

    init(
        dataTypeExample: T,
        moduleId: String,
        channelName: String,
        toModuleManagerQueue: AsyncStream<Claidservice_DataPackage>.Continuation
    ) {
        self.moduleId = moduleId
        self.channelName = channelName
      
     
        self.mutator = TypeMapping.getMutator(type(of: dataTypeExample))
        self.toModuleManagerQueue = toModuleManagerQueue
    }

    /// Posts data to the `AsyncStream` with the current timestamp
    func post(_ data: T) async {
        await post(data, timestamp: Date()) // Use current time
    }

    /// Posts data to the `AsyncStream` with a specified `Date` timestamp
    func post(_ data: T, timestamp: Date) async {
        var dataPackage = Claidservice_DataPackage()
        dataPackage.sourceModule = moduleId
        dataPackage.channel = channelName
        dataPackage.unixTimestampMs = UInt64(timestamp.timeIntervalSince1970 * 1000) // Convert to milliseconds

        // Apply the mutator to set the package payload
        dataPackage = mutator.setPackagePayload(packet: dataPackage, value: data)

        // Send to the `AsyncStream`
        toModuleManagerQueue.yield(dataPackage)
    }
}
