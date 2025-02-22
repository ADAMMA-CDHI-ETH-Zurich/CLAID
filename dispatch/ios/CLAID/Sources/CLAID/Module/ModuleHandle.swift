//
//  ModuleBackend.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//

public class ModuleHandle {
    public let dispatcher = RunnableDispatcher()
    public var tasks: [String: Task<Void, Never>] = [:]
    public var counter: Int = 0  // A variable to be modified by tasks
    public var id: String = ""
    public var type: String = ""
    public var subscriberPublisher: ChannelSubscriberPublisher?
    public var remoteFunctionHandler: RemoteFunctionHandler?
    public var properties: Properties?
    public var isInitialized: Bool = false
}
