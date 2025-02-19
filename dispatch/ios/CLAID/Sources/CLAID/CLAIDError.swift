//
//  CLAIDError.swift
//  CLAID
//
//  Created by Patrick Langer on 19.02.2025.
//

import Foundation

/// CLAID Error struct for handling user-defined errors.
public struct CLAIDError: Error, LocalizedError {
    public let message: String
    
    public init(_ message: String) {
        self.message = message
    }
    
    /// Provides a localized description for the error.
    public var errorDescription: String? {
        return message
    }
}

/// Helper extension for easy error throwing.
public extension CLAIDError {
    static func error(_ message: String) -> CLAIDError {
        return CLAIDError(message)
    }
}
