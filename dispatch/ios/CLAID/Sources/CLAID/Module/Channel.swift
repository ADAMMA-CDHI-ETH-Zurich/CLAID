import Foundation

enum ChannelAccessRights {
    case read, write, readWrite, none
}

public actor Channel<T: Sendable> {
    private let channelId: String
    private let accessRights: ChannelAccessRights
    
    private var publisher: Publisher<T>?
    private var subscriber: Subscriber<T>?
    private var callbackRegistered = false
    private var valid = false

    /// Determines if the channel has read access
    private func canRead() -> Bool {
        return accessRights == .read || accessRights == .readWrite
    }

    /// Determines if the channel has write access
    private func canWrite() -> Bool {
        return accessRights == .write || accessRights == .readWrite
    }

    /// Constructor for **published** channels
    init(channelId: String, publisher: Publisher<T>) {
        self.channelId = channelId
        self.accessRights = .write
        self.publisher = publisher
        self.valid = true
    }

    /// Constructor for **subscribed** channels
    init(channelId: String, subscriber: Subscriber<T>) {
        self.channelId = channelId
        self.accessRights = .read
        self.subscriber = subscriber
        self.callbackRegistered = subscriber != nil
        self.valid = true
    }

    /// Private constructor for **invalid** channels
    private init(channelId: String) {
        self.channelId = channelId
        self.accessRights = .none
        self.valid = false
    }

    /// Factory method to create an **invalid** channel
    public static func newInvalidChannel(channelId: String) -> Channel<T> {
        return Channel(channelId: channelId)
    }

    /// Posts data to the channel with the current timestamp
    public func post(_ data: T) async {
        guard canWrite() else {
            let msg = "Tried to post data to channel \"\(self.channelId)\", but it was not published before."

            Logger.logFatal(msg)
            return
        }
        await publisher?.post(data, timestamp: Date.now)
    }

    /// Posts data to the channel with a custom timestamp
    public func post(_ data: T, date: Date) async {
        guard canWrite() else {
            let msg = "Tried to post data to channel \"\(self.channelId)\", but it was not published before."

            
            Logger.logFatal(msg)
            return
        }
        await publisher?.post(data, timestamp: date)
    }

    /// Returns the channel ID
    func getChannelId() -> String {
        return self.channelId
    }
}

