import GRPCCore
import Foundation
import GRPCNIOTransportHTTP2
import os

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

    func getModuleList(registeredModuleClasses: [String]) async throws -> Claidservice_ModuleListResponse {
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
    
    
}
