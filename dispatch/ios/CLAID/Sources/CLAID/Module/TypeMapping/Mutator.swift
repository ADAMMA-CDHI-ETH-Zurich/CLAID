//
//  Mutator.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//


import Foundation

class Mutator<T>: AbstractMutator {
    private let setter: (Claidservice_DataPackage, T) -> Claidservice_DataPackage
    private let getter: (Claidservice_DataPackage) -> T

    init(setter: @escaping (Claidservice_DataPackage, T) -> Claidservice_DataPackage,
         getter: @escaping (Claidservice_DataPackage) -> T) {
        self.setter = setter
        self.getter = getter
    }

    /// Sets the payload in an immutable `Claidservice_DataPackage` and returns a new instance
    func setPackagePayload(packet: Claidservice_DataPackage, value: T) -> Claidservice_DataPackage {
        return setter(packet, value)
    }

    /// Retrieves the payload from `Claidservice_DataPackage`
    func getPackagePayload(_ packet: Claidservice_DataPackage) -> T {
        return getter(packet)
    }

    /// Casts an object to type `T`
    func cast(_ object: Any) -> T? {
        return object as? T
    }

    /// Overrides `AbstractMutator` method to set payload from `Any` object
    func setPackagePayloadFromObject(_ packet: Claidservice_DataPackage, object: Any) -> Claidservice_DataPackage {
        guard let value = object as? T else {
            fatalError("Type mismatch: Expected \(T.self), but got \(type(of: object))")
        }
        return setPackagePayload(packet: packet, value: value)
    }

    /// Overrides `AbstractMutator` method to retrieve payload as `Any`
    func getPackagePayloadAsObject(_ packet: Claidservice_DataPackage) -> Any {
        return getPackagePayload(packet) as Any
    }
}
