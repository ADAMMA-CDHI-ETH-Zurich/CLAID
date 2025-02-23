//
//  Subscriber.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//


import Foundation

actor Subscriber<T: Sendable>: AbstractSubscriber {
    private let callback: (ChannelData<T>) -> Void
    private let callbackDispatcher: RunnableDispatcher
    private let mutator: Mutator<T>

    init(dataTypeExample: T, callback: @escaping (ChannelData<T>) -> Void, callbackDispatcher: RunnableDispatcher) {
        self.callback = callback
        self.callbackDispatcher = callbackDispatcher
        self.mutator = TypeMapping.getMutator(type(of: dataTypeExample))
    }

    /// Converts `Int64` UNIX timestamp (milliseconds) to `Date`
    private func convertUnixTimestampToDate(_ timestampMs: UInt64) -> Date {
        return Date(timeIntervalSince1970: TimeInterval(timestampMs) / 1000)
    }

    /// Calls the callback function on a background dispatcher
    private func invokeCallback(_ data: ChannelData<T>) async {
        
        await callback(data)
        
    }

    /// Handles new incoming data
    func onNewData(dataPackage: Claidservice_DataPackage) async {
        let value: T = mutator.getPackagePayload(dataPackage)

        let timestamp = convertUnixTimestampToDate(dataPackage.unixTimestampMs)
        let channelData = ChannelData(
            data: value,
            timestamp: timestamp,
            userToken: dataPackage.sourceUserToken
        )

        await invokeCallback(channelData)
    }
}
