//
//  AbstractSubscriber.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//

public protocol AbstractSubscriber : Actor
{
    func onNewData(dataPackage: Claidservice_DataPackage) async
}
