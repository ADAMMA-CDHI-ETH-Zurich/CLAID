//
//  Properties.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//
import SwiftProtobuf
public class Properties {
    
    let properties: Google_Protobuf_Struct
    
    public init(properties: Google_Protobuf_Struct) {
        self.properties = properties
    }
}
