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
    
    func makeControlRuntimePing() async -> Claidservice_DataPackage {
        var dataPackage = Claidservice_DataPackage()
        var controlPkg = Claidservice_ControlPackage()
        controlPkg.ctrlType = .ctrlRuntimePing
        controlPkg.runtime = .swift  // Or whichever runtime
        dataPackage.controlVal = controlPkg
        return dataPackage
    }

    
    public func sendReceivePackages(
        inStream: AsyncThrowingStream<Claidservice_DataPackage, Error>,
        outContinuation: AsyncThrowingStream<Claidservice_DataPackage, Error>.Continuation
    ) async throws -> Bool {
        
        // Example handshake actor, if needed:
        let pingPongWaiter = CtrlRuntimePingPongWaiter()
        
        // Build the request that will consume from inStream
        let request = StreamingClientRequest<Claidservice_DataPackage>(producer: { writer in
            // 1) (Optional) Send initial ping
            try await writer.write(self.makeControlRuntimePing())
            await pingPongWaiter.waitForPong()
            
            // 2) Send additional packages from the client→server inStream
            for try await package in inStream {
                try await writer.write(package)
            }
        })
        
        // Perform the bidirectional call, capturing server→client messages:
        let _ = try await stub.sendReceivePackages(request: request) { fromMiddlewareStream in
            do {
                for try await msg in fromMiddlewareStream.messages {
                    // (Optional) check handshake
                    if msg.hasControlVal && msg.controlVal.ctrlType == .ctrlRuntimePing {
                        await pingPongWaiter.gotPong()
                    }
                    
                    // Push the message to outContinuation
                    outContinuation.yield(msg)
                }
            } catch {
                // Optionally throw an error into the continuation
                outContinuation.finish(throwing: error)
            }
            
            // When server finishes or an error occurs, close the out stream
            outContinuation.finish()
        }

        return true
    }


}


actor CtrlRuntimePingPongWaiter {
    private var continuation: CheckedContinuation<Void, Never>?

    /// Suspends until `gotPong()` is called.
    func waitForPong() async {
        await withCheckedContinuation { cont in
            self.continuation = cont
        }
    }

    /// Resumes the `waitForPong()` call, allowing it to proceed.
    func gotPong() {
        continuation?.resume()
        continuation = nil
    }
}
