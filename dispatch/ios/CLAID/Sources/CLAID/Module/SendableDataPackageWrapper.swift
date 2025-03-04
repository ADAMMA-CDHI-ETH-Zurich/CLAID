//
//  Untitled.swift
//  CLAID
//
//  Created by Patrick Langer on 22.02.2025.
//

actor SendableDataPackageWrapper {
    public let dataPackage: Claidservice_DataPackage
    
    init(dataPackage: Claidservice_DataPackage) {
        self.dataPackage = dataPackage
    }
}
