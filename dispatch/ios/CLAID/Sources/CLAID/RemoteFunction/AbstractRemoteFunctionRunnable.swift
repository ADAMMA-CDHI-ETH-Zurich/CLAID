//
//  AbstractRemoteFunctionRunnable.swift
//  CLAID
//
//  Created by Patrick Langer on 23.02.2025.
//

protocol AbstractRemoteFunctionRunnable {
    func executeRemoteFunctionRequest(_ rpcRequest: Claidservice_DataPackage) -> Claidservice_DataPackage?
}
