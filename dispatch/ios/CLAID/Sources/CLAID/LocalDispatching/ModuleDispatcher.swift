import GRPCCore
import Foundation
import GRPCNIOTransportHTTP2
import os

import SwiftProtobuf

actor ModuleDispatcher {
    
    private let channel: GRPCClient<HTTP2ClientTransport.Posix>
    private let stub: Claidservice_ClaidService.Client<HTTP2ClientTransport.Posix>
    private let socketPath: String

    init(socketPath: String) async throws {
        self.socketPath = socketPath
        self.channel = try await Self.createGRPCClient(socketPath: socketPath)
        self.stub = .init(wrapping: channel)
        runClient()
    }
    
    private static func createGRPCClient(socketPath: String) async throws -> GRPCClient<HTTP2ClientTransport.Posix> {

        var normalizedPath = socketPath
        let unixPrefix = "unix://" // Middleware requires this prefix for unix domain sockets, but we don't. Otherwise our path is wrong.

        if normalizedPath.hasPrefix(unixPrefix) {
           normalizedPath.removeFirst(unixPrefix.count)
        }
        
        guard let transport = try? HTTP2ClientTransport.Posix.http2NIOPosix(
            target: .unixDomainSocket(path: normalizedPath),
            transportSecurity: .plaintext
        ) else {
            throw NSError(domain: "TransportError", code: 1, userInfo: [NSLocalizedDescriptionKey: "Failed to create transport"])
        }
        return GRPCClient(transport: transport)
    }

    private func runClient() {
        Task {
            do {
                Logger.logInfo("Running connections")
                try await self.channel.runConnections()
            } catch {
                Logger.logError("Client closed with status: \(error.localizedDescription)")
            }
        }
    }
    
    func stopClient() {
        Logger.logInfo("Initiating graceful shutdown")
        self.channel.beginGracefulShutdown()
    }

    public func getModuleList(registeredModuleClasses: [String]) async throws -> Claidservice_ModuleListResponse {
        var request = Claidservice_ModuleListRequest()
        request.runtime = Claidservice_Runtime.swift
        request.supportedModuleClasses = registeredModuleClasses
        request.moduleAnnotations = [:]
        
        print("Sending ModuleListRequest: \(request)")
        Logger.logInfo("Swift Runtime: Calling getModuleList(...)")
        
        do {
            let response = try await self.stub.getModuleList(request: .init(message: request))
            print("Response received: \(response)")
            return response
        } catch {
            print("Error in getModuleList: \(error.localizedDescription)")
            throw error
        }
    }
    
    public func initRuntime(channelExamplePackages: [String: [Claidservice_DataPackage]]) async throws -> Bool {
        var initRuntimeRequest = Claidservice_InitRuntimeRequest()

        for (moduleId, dataPackages) in channelExamplePackages {
            var moduleChannels = Claidservice_InitRuntimeRequest.ModuleChannels()
            moduleChannels.moduleID = moduleId
            moduleChannels.channelPackets.append(contentsOf: dataPackages)
            print("Adding \(dataPackages)")
            initRuntimeRequest.modules.append(moduleChannels)
        }

        initRuntimeRequest.runtime = .swift  // Assuming equivalent Swift enum exists

        var resultReceived = false

        Logger.logInfo("Swift Runtime: Calling initRuntime(...)")
        let response = try await stub.initRuntime(request: .init(message: initRuntimeRequest))


        return true
    }
    
    public func sendReceivePackagesdd(inConsumer: @escaping (DataPackage) -> Void) async throws -> Bool {
        
        
        
        let packagesToSend = [
            Claidservice_DataPackage()
        ]
            
        // Create the streaming request
        let request = StreamingClientRequest<Claidservice_DataPackage>(
            producer: { writer in
                // Write messages to the server
                for package in packagesToSend {
                    try await writer.write(package)
                }
                // Closing the writer ends the client->server stream
            }
        )
        
        // Make the bidirectional streaming call
        let call = try await stub.sendReceivePackages(request: request) { writer in
            
        }
            // Handle incoming packages from server
            for try await receivedPackage in call.responseStream {
                print("Received package: \(receivedPackage)")
            }
        
        
        self.inConsumer = inConsumer

        guard inConsumer != nil else {
            Logger.logFatal("Invalid argument in ModuleDispatcher::sendReceivePackages. Provided consumer is nil.")
            return false
        }

        self.waitForInputStreamCancelled = false
        self.inputStreamCancelled = false

        self.inStream = makeInputStreamObserver(
            onNext: { dataPackage in self.onMiddlewareStreamPackageReceived(dataPackage) },
            onError: { error in self.onMiddlewareStreamError(error) },
            onCompleted: { self.onMiddlewareStreamCompleted() }
        )
        
        let inQueue = RPCAsyncSequence<Claidservice_DataPackage, any Error>(wrapping: <#_#>)
        
        let writer = RPCWriter<Claidservice_DataPackage>{
            return Claidservice_DataPackage()
        }
        let value = StreamingClientRequest.init(of: Claidservice_DataPackage.self, producer: <#T##(RPCWriter<Claidservice_DataPackage>) async throws -> Void#>)
        
        stub.sendReceivePackages(request: {continuation in
        }, onResponse: <#T##(StreamingClientResponse<Claidservice_DataPackage>) async throws -> Sendable#>)
        
        func routeChat(
          request: RPCAsyncSequence<Routeguide_RouteNote, any Error>,
          response: RPCWriter<Routeguide_RouteNote>,
          context: ServerContext
        ) async throws {
        }
        
        let streamingRequest = StreamingClientRequest<Claidservice_DataPackage>(, producer: <#@Sendable (RPCWriter<Claidservice_DataPackage>) async throws -> Void#>)

        stub.sendReceivePackages(request: StreamingClientRequest<Claidservice_DataPackage>, onResponse: <#T##(StreamingClientResponse<Claidservice_DataPackage>) async throws -> Sendable#>)
        self.outStream = stub.sendReceivePackages(self.inStream)

        guard self.outStream != nil else {
            return false
        }

        self.waitingForPingResponse = true
        let pingReq = makeControlRuntimePing()
        self.outStream?.onNext(pingReq)

        awaitPingPackage()

        return true
    }

    
}
