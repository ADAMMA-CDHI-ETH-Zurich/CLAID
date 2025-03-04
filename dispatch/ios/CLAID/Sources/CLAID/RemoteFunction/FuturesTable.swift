//
//  FuturesTable.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//



public actor FuturesTable {
    private var futures: [FutureUniqueIdentifier: AbstractFuture] = [:]
    
    func addFuture(_ future: AbstractFuture) async {
        await self.futures[future.getUniqueIdentifier()] = future
    }
    
    func removeFuture(_ futureIdentifier: FutureUniqueIdentifier) async -> Bool {
        if let _ = self.futures.removeValue(forKey: futureIdentifier) {
            return true
        }
        return false
    }
    
    func lookupFuture(_ uniqueIdentifier: FutureUniqueIdentifier) async -> AbstractFuture? {
        return await self.futures[uniqueIdentifier]
    }
}
