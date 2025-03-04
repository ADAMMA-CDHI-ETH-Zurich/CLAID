//
//  ChannelData.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//


import Foundation

public actor ChannelData<T: Sendable> : Sendable{
    private let data: T
    let timestamp: Date
    let userToken: String
    let valid: Bool

    init(data: T, timestamp: Date, userToken: String) {
        self.data = data
        self.timestamp = timestamp
        self.userToken = userToken
        self.valid = true
    }
    
    func getData() async -> T {
        return data
    }

    /// Returns whether the data is valid
    func isValid() async -> Bool {
        return valid
    }
}
