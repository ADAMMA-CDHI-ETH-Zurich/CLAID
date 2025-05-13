import Foundation

public actor ChannelSubscriberPublisher {
    // Stores example data packages for each module
    private var examplePackagesForEachModule: [String: [Claidservice_DataPackage]] = [:]

    // Maps (channelName, moduleId) -> List of subscribers
    private var moduleChannelsSubscriberMap: [String: [String: [AbstractSubscriber]]] = [:]

    let toModuleManagerQueue: AsyncStream<Claidservice_DataPackage>.Continuation

    init(toModuleManagerQueue: AsyncStream<Claidservice_DataPackage>.Continuation) {
       self.toModuleManagerQueue = toModuleManagerQueue
   }

    /// Prepares an example `DataPackage` for a module publishing or subscribing to a channel
    private func prepareExamplePackage<T>(dataTypeExample: T, moduleId: String, channelName: String, isPublisher: Bool) -> Claidservice_DataPackage {
        var dataPackage = Claidservice_DataPackage()
        
        if isPublisher {
            dataPackage.sourceModule = moduleId
        } else {
            dataPackage.targetModule = moduleId
        }

        dataPackage.channel = channelName
        print("Preparing example package for channel: \(channelName) with data type: \(type(of: dataTypeExample))")

        let mutator = TypeMapping.getMutator(type(of: dataTypeExample))
        dataPackage = mutator.setPackagePayload(packet: dataPackage, value: dataTypeExample)

        return dataPackage
    }

    /// Publishes a new channel for a module
    func publish<T: Sendable>(dataTypeExample: T, moduleId: String, channelName: String) async -> Channel<T> {
        let examplePackage = prepareExamplePackage(dataTypeExample: dataTypeExample, moduleId: moduleId, channelName: channelName, isPublisher: true)

        print("Inserting package for Module \(moduleId)")
        examplePackagesForEachModule[moduleId, default: []].append(examplePackage)

        let publisher = Publisher(
            dataTypeExample: dataTypeExample,
            moduleId: moduleId,
            channelName: channelName,
            toModuleManagerQueue: toModuleManagerQueue
        )
        return Channel(channelId: channelName, publisher: publisher)
    }

    /// Subscribes a module to a channel
    func subscribe<T>(dataTypeExample: T, moduleId: String, channelName: String, subscriber: Subscriber<T>) async -> Channel<T> {
        let examplePackage = prepareExamplePackage(dataTypeExample: dataTypeExample, moduleId: moduleId, channelName: channelName, isPublisher: false)

        examplePackagesForEachModule[moduleId, default: []].append(examplePackage)
        insertSubscriber(channelName: channelName, moduleId: moduleId, subscriber: subscriber)

        return Channel(channelId: channelName, subscriber: subscriber)
    }

    /// Inserts a subscriber into the subscription map
    private func insertSubscriber(channelName: String, moduleId: String, subscriber: AbstractSubscriber) {
        moduleChannelsSubscriberMap[channelName, default: [:]][moduleId, default: []].append(subscriber)
    }

    /// Retrieves the list of subscribers for a module on a given channel
    func getSubscriberInstancesOfModule(channelName: String, moduleId: String) -> [AbstractSubscriber] {
        return moduleChannelsSubscriberMap[channelName]?[moduleId] ?? []
    }

    /// Retrieves stored template `DataPackages` for a module
    func getChannelTemplatePackagesForModule(moduleId: String) -> [Claidservice_DataPackage] {
        return examplePackagesForEachModule[moduleId] ?? []
    }

    /// Checks if a `DataPackage` is compatible with a channel for a given receiver module
    func isDataPackageCompatibleWithChannel(dataPackage: Claidservice_DataPackage, receiverModule: String) async -> Bool {
        guard let examplePackages = examplePackagesForEachModule[receiverModule] else { return false }

        for templatePackage in examplePackages where templatePackage.channel == dataPackage.channel {
            return templatePackage.payload.messageType == dataPackage.payload.messageType
        }
        return false
    }

    /// Retrieves the payload data type of a channel for a given receiver module
    func getPayloadCaseOfChannel(channelName: String, receiverModule: String) -> String {
        guard let examplePackages = examplePackagesForEachModule[receiverModule] else { return "" }

        for templatePackage in examplePackages where templatePackage.channel == channelName {
            return templatePackage.payload.messageType
        }
        return ""
    }

    /// Sends a `DataPackage` to the async stream (used by `ModuleManager`)
    func sendToModuleManager(_ dataPackage: Claidservice_DataPackage) {
        toModuleManagerQueue.yield(dataPackage)
    }

    /// Resets all stored channels and subscribers
    func reset() {
        examplePackagesForEachModule.removeAll()
        moduleChannelsSubscriberMap.removeAll()
    }
}
