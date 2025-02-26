//
//  TypeMapping.swift
//  CLAID
//
//  Created by Patrick Langer on 21.02.2025.
//


import Foundation
import SwiftProtobuf
class TypeMapping {
    
    /// Creates a copy of a `Claidservice_DataPackage` while preserving its properties
    static func dataPackageBuilderCopy(_ p: Claidservice_DataPackage) -> Claidservice_DataPackage {
        var builder = Claidservice_DataPackage()
        builder.id = p.id
        builder.channel = p.channel
        builder.sourceModule = p.sourceModule
        builder.targetModule = p.targetModule
        builder.sourceHost = p.sourceHost
        builder.targetHost = p.targetHost
        builder.sourceUserToken = p.sourceUserToken
        builder.targetUserToken = p.targetUserToken
        builder.unixTimestampMs = p.unixTimestampMs
        builder.deviceID = p.deviceID
        
        if p.hasControlVal {
            builder.controlVal = p.controlVal
        }
        
        
        builder.tracePoints = p.tracePoints // Preserve trace points
        return builder
    }
    
    /// Returns an instance of a Protobuf message using its `newBuilder()` method.
    static func getProtoMessageInstance<T: SwiftProtobuf.Message>(_ type: T.Type) -> T? {
        return T()
    }
    
    /// Generic function to create an instance of a given type `T`
    static func getNewInstance<T>(_ type: T.Type) -> T? {
        if let protobufType = type as? SwiftProtobuf.Message.Type {
            return protobufType.init() as? T
        }
        
        return nil // Avoid assuming `T` has a default initializer
    }
    
    
    
    private static func getProtoCodec(_ msg: SwiftProtobuf.Message) -> ProtoCodec {
        return ProtoCodec(msg: type(of: msg))
    }
    
    /// Sets a Protobuf message as the payload of a `Claidservice_DataPackage`
    private static func setProtoPayload<T: SwiftProtobuf.Message>(_ packet: Claidservice_DataPackage, _ protoValue: T) -> Claidservice_DataPackage {
        var packetCopy = dataPackageBuilderCopy(packet)
        let protoCodec = getProtoCodec(protoValue)
        let blob = protoCodec.encode(protoValue)
        packetCopy.payload = blob
        return packetCopy
    }
    
    /// Extracts a Protobuf message from a `Claidservice_DataPackage`
    private static func getProtoPayload<T: SwiftProtobuf.Message>(_ packet: Claidservice_DataPackage, _ instance: T) -> T? {
        let protoCodec = getProtoCodec(instance)
        
        guard packet.payload.messageType != "" else {
            print("Invalid package: missing payload message type!")
            return nil
        }
        
        return protoCodec.decode(from: packet.payload) as? T
    }
    
    /// Returns a `Mutator<T>` based on the given `DataType`
    static func getMutator<T>(_ type: T.Type) -> Mutator<T> {
        
        if type == Int.self || type == Int64.self || type == Int32.self || type == UInt.self || type == UInt64.self || type == UInt32.self {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_IntVal()
                    val.val = Int64(value as! Int)
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    
                    let val = getProtoPayload(packet, Claidservice_IntVal())!
                    return Int(truncatingIfNeeded: val.val) as! T
                }
            )
        }
        
        if type == Float.self || type == Double.self {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_DoubleVal()
                    val.val = value as! Double
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    let val = getProtoPayload(packet, Claidservice_DoubleVal())!
                    return val.val as! T
                }
            )
        }
        
        if type == Bool.self {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_BoolVal()
                    val.val = value as! Bool
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    let val = getProtoPayload(packet, Claidservice_BoolVal())!
                    return val.val as! T
                }
            )
        }
        
        if type == String.self {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_StringVal()
                    val.val = value as! String
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    let val = getProtoPayload(packet, Claidservice_StringVal())!
                    return val.val as! T
                }
            )
        }
        
        if type is [Double].Type {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_NumberArray()
                    val.val = value as! [Double]
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    let val = getProtoPayload(packet, Claidservice_NumberArray())!
                    return val.val as! T
                }
            )
        }
        
        if type is [String].Type {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_StringArray()
                    val.val = value as! [String]
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    let val = getProtoPayload(packet, Claidservice_StringArray())!
                    return val.val as! T
                }
            )
        }
        
        if type is Dictionary<String,Double>.Type {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_NumberMap()
                    val.val = value as! Dictionary<String,Double>
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    let val = getProtoPayload(packet, Claidservice_NumberMap())!
                    return val.val as! T
                }
            )
        }
        
        if type is Dictionary<String,String>.Type {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    var val = Claidservice_StringMap()
                    val.val = value as! Dictionary<String,String>
                    return setProtoPayload(builder, val)
                },
                getter: { packet in
                    let val = getProtoPayload(packet, Claidservice_StringMap())!
                    return val.val as! T
                }
            )
        }
        
        if let messageType = type as? Message.Type {
            return Mutator<T>(
                setter: { packet, value in
                    var builder = dataPackageBuilderCopy(packet)
                    let codec = TypeMapping.getProtoCodec(value as! SwiftProtobuf.Message)
                    builder.payload = codec.encode(value as! SwiftProtobuf.Message)
                    return builder
                },
                getter: { packet in
                    let instance = messageType.init()
                    let codec = TypeMapping.getProtoCodec(instance)
                    return codec.decode(from: packet.payload) as! T
                }
            )
        }
        
        fatalError("Unsupported type in TypeMapping: \(type)")
    }

}
