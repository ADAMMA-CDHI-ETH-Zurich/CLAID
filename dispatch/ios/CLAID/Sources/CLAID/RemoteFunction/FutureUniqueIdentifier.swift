//
//  FutureUniqueIdentifier.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

import Foundation

public final class FutureUniqueIdentifier: Comparable, Sendable, Hashable {
    
    private static let uniqueIdProfier = UniqueIdProvider()
    
    private let identifier: String
    
    private init(id: String) {
        self.identifier = id
    }
    
    static func makeUniqueIdentifier() async -> FutureUniqueIdentifier {
        let id = await uniqueIdProfier.nextUniqueId()
        
        let timestamp = Int64(Date().timeIntervalSince1970 * 1000)
        let identifier = "CLAID_SWIFT_\(id)_\(timestamp)"
        
        return FutureUniqueIdentifier(id: identifier)
    }
    
    static func fromString(_ identifier: String) -> FutureUniqueIdentifier {
        return FutureUniqueIdentifier(id: identifier)
    }
    
    func toString() -> String {
        return self.identifier
    }
    
    public static func < (lhs: FutureUniqueIdentifier, rhs: FutureUniqueIdentifier) -> Bool {
        return lhs.identifier < rhs.identifier
    }
    
    public static func == (lhs: FutureUniqueIdentifier, rhs: FutureUniqueIdentifier) -> Bool {
        return lhs.identifier == rhs.identifier
    }
    
    public func hash(into hasher: inout Hasher) {
        hasher.combine(identifier)
    }
}
