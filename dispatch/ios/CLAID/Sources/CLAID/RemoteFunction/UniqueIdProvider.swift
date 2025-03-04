//
//  UniqueIdProvider.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//


public actor UniqueIdProvider {
    private var currentId: UInt64 = 0
    
    public func nextUniqueId() -> UInt64 {
        currentId += 1
        return currentId - 1
    }
}
