//
//  FutureHandler.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//


public actor FutureHandler {
    
    // The table that stores currently open futures
    private let openFutures = FuturesTable()
    
    public init() {}
    
    
    /// Creates and registers a new `Future<T>` with a freshly generated `FutureUniqueIdentifier`.
    ///
    /// - Parameter dataTypeExample: An instance of `T` just to illustrate how we might obtain a `Mutator<T>`
    ///                              (matching your `Future<T>` constructor requirement).
    /// - Returns: The newly created `Future<T>`.
    public func registerNewFuture<T: Sendable>(dataTypeExample: T) async -> Future<T> {
        // 1. Generate a unique identifier
        let uniqueIdentifier = await FutureUniqueIdentifier.makeUniqueIdentifier()
        
        // 2. Create a new Future<T>
        let future = Future<T>(
            futuresTable: openFutures,
            uniqueIdentifier: uniqueIdentifier,
            dataTypeExample: dataTypeExample
        )
        
        // 3. Store (register) the future in the FuturesTable
        //    Make sure your `FuturesTable` has an `addFuture(_:)` or similar method.
        await openFutures.addFuture(future as AbstractFuture)
        
        // 4. Return the new future
        return future
    }
    
    
    /// Look up a stored future by its unique identifier.
    ///
    /// - Returns: An `AbstractFuture?` if found, or `nil` if no future with `identifier` was found.
    public func lookupFuture(identifier: FutureUniqueIdentifier) async -> AbstractFuture? {
        // Forward the request to openFutures
        return await openFutures.lookupFuture(identifier)
    }
}
