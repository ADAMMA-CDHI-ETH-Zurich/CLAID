//
//  AbstractMutator.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//


protocol AbstractMutator : Sendable {
    /// Sets the package payload from an `Any` object.
    func setPackagePayloadFromObject(_ packet: Claidservice_DataPackage, object: Any) -> Claidservice_DataPackage

    /// Retrieves the package payload as an `Any` object.
    func getPackagePayloadAsObject(_ packet: Claidservice_DataPackage) -> Any
}
