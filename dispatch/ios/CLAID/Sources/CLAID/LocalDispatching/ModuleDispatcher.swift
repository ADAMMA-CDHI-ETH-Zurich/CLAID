import GRPCCore
import Foundation
import GRPCNIOTransportHTTP2
import os

import SwiftProtobuf

actor ModuleDispatcher {
    
    private let channel: GRPCClient<HTTP2ClientTransport.Posix>
    private let stub: Claidservice_ClaidService.Client<HTTP2ClientTransport.Posix>
    private let socketPath: String

    var fromMiddlewareContinuation: AsyncStream<Claidservice_DataPackage>.Continuation!
    public let fromMiddlewareStream: AsyncStream<Claidservice_DataPackage>

    var toMiddlewareContinuation: AsyncStream<Claidservice_DataPackage>.Continuation!
    public let toMiddlewareStream: AsyncStream<Claidservice_DataPackage>

    
    init(socketPath: String) async throws {
        self.socketPath = socketPath
        self.channel = try await Self.createGRPCClient(socketPath: socketPath)
        self.stub = .init(wrapping: channel)
        
        func makePackageStreamAndContinuation()
            -> (stream: AsyncStream<Claidservice_DataPackage>,
                continuation: AsyncStream<Claidservice_DataPackage>.Continuation)
        {
            var continuation: AsyncStream<Claidservice_DataPackage>.Continuation!
            let stream = AsyncStream<Claidservice_DataPackage> { cont in
                continuation = cont
            }
            return (stream, continuation)
        }
        
        (self.fromMiddlewareStream, self.fromMiddlewareContinuation) = makePackageStreamAndContinuation()
        (self.toMiddlewareStream, self.toMiddlewareContinuation) = makePackageStreamAndContinuation()

        runClient()
    }
    
    private static func splitHostAndPort(input: String) async throws -> (host: String, port: Int?) {
        let components = input.split(separator: ":", maxSplits: 1, omittingEmptySubsequences: true)
        
        guard !components.isEmpty else {
            throw CLAIDError("Input string is empty or invalid")
        }
        
        let host = String(components[0])
        let port = components.count > 1 ? Int(components[1]) : nil
        
        return (host, port)
    }

    
    private static func createGRPCClient(socketPath: String) async throws -> GRPCClient<HTTP2ClientTransport.Posix> {

        var normalizedPath = socketPath
        let unixPrefix = "unix://" // Middleware requires this prefix for unix domain sockets, but we don't. Otherwise our path is wrong.

        let dnsPrefix = "dns:"

        if normalizedPath.hasPrefix(unixPrefix) {
            normalizedPath.removeFirst(unixPrefix.count)
            guard let transport = try? HTTP2ClientTransport.Posix.http2NIOPosix(
                target: .unixDomainSocket(path: normalizedPath),
                transportSecurity: .plaintext
            ) else {
                throw NSError(domain: "TransportError", code: 1, userInfo: [NSLocalizedDescriptionKey: "Failed to create transport"])
            }
            return GRPCClient(transport: transport)
        } else {
            if normalizedPath.hasPrefix(dnsPrefix) {
                normalizedPath.removeFirst(dnsPrefix.count)
            }
            
            let (host, port) = try await splitHostAndPort(input: normalizedPath)
            guard let transport = try? HTTP2ClientTransport.Posix.http2NIOPosix(
                target: .dns(host: host, port: port),
                transportSecurity: .plaintext
            ) else {
                throw NSError(domain: "TransportError", code: 1, userInfo: [NSLocalizedDescriptionKey: "Failed to create transport"])
            }
            return GRPCClient(transport: transport)
        }
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
            initRuntimeRequest.modules.append(moduleChannels)
        }

        initRuntimeRequest.runtime = .swift  // Assuming equivalent Swift enum exists

        var resultReceived = false

        Logger.logInfo("Swift Runtime: Calling initRuntime(...) \(initRuntimeRequest)")
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

    
    public func sendReceivePackages() async throws -> Bool {
        
        // Example handshake actor, if needed:
        let pingPongWaiter = CtrlRuntimePingPongWaiter()
        
        // Build the request that will consume from inStream
        let request = StreamingClientRequest<Claidservice_DataPackage>(producer: { writer in
            // Send initial ping package
            try await writer.write(self.makeControlRuntimePing())
            await pingPongWaiter.waitForPong()
            
            for try await package in self.toMiddlewareStream {
                Logger.logInfo("Writing package \(package)")
                try await writer.write(package)
            }
        })
        
        // Perform the bidirectional call, capturing server→client messages:
        let result = try await stub.sendReceivePackages(request: request) { inStream in
            do {
                for try await msg in inStream.messages {
                    Logger.logInfo("awaiting pong")
                    if await pingPongWaiter.isWaiting() {
                        Logger.logInfo("got pong 0")

                        if msg.hasControlVal && msg.controlVal.ctrlType == .ctrlRuntimePing {
                            Logger.logInfo("got pong 1")
                            await pingPongWaiter.gotPong()
                            Logger.logInfo("got pong 2")

                        } else {
                            throw CLAIDError("Received invalid package from middleware while waiting for ping response. Got \(msg) but awaited Control message with CTRL_RUNTIME_PING.")
                        }
                    } else {
                        Logger.logInfo("On package received")
                        await self.onPackageReceivedFromMiddleware(msg)
                    }
                }
            } catch {
                // Optionally throw an error into the continuation
                Logger.logFatal("Error in server stream: \(error)")
                await self.fromMiddlewareContinuation.finish()
            }
            
            // When server finishes or an error occurs, close the out stream
            await self.fromMiddlewareContinuation.finish()
        }

        return true
    }
    
    func onPackageReceivedFromMiddleware(_ package: Claidservice_DataPackage) async {
        // Optional here we could handle further control package.
        Logger.logInfo("Yielding!")

        await self.fromMiddlewareContinuation.yield(package)
    }
    
    public func getToMiddlewareContinuation() async -> AsyncStream<Claidservice_DataPackage>.Continuation {
        return self.toMiddlewareContinuation
    }
    
    public func getFromMiddlewareStream() async -> AsyncStream<Claidservice_DataPackage> {
        return self.fromMiddlewareStream
    }

}


actor CtrlRuntimePingPongWaiter {
    private var continuation: CheckedContinuation<Void, Never>?
    private var waitingForPingResponse = false
    
    init() {
        waitingForPingResponse = true
    }
    
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
        waitingForPingResponse = false
    }
    
    public func isWaiting() -> Bool {
        return waitingForPingResponse
    }
}
