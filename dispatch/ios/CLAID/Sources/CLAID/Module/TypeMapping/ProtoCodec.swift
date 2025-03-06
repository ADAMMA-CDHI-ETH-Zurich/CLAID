//
//  ProtoCodec.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//


import Foundation
import SwiftProtobuf

class ProtoCodec {
    private let fullName: String
    private let msg: SwiftProtobuf.Message.Type

    init(msg: SwiftProtobuf.Message.Type) {
        self.msg = msg
        self.fullName = String(describing: msg).replacingOccurrences(of: "Claidservice_", with: "")
    }

    func encode(_ protoMessage: SwiftProtobuf.Message) -> Claidservice_Blob {
        do {
            let serializedData = try protoMessage.serializedData()
            var blob = Claidservice_Blob()
            blob.codec = .proto
            blob.payload = serializedData
            blob.messageType = fullName
            return blob
        } catch {
            print("Encoding failed: \(error)")
            return Claidservice_Blob()
        }
    }

    func decode(from blob: Claidservice_Blob) -> SwiftProtobuf.Message? {
        do {
            var message = try msg.init(serializedData: blob.payload)
            return message
        } catch {
            print("Decoding failed: \(error)")
            return nil
        }
    }
}
