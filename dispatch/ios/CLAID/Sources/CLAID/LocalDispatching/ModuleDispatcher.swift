import GRPCCore
import Foundation
import GRPCNIOTransportHTTP2

class ModuleDispatcher {

    let client: GRPCClient<HTTP2ClientTransport.Posix>
    
    init() async throws{
        guard let transport = try? HTTP2ClientTransport.Posix.http2NIOPosix(
           target: .unixDomainSocket(path: "unix://claid_local.grpc"),
           transportSecurity: .plaintext
       ) else {
           throw NSError(domain: "TransportError", code: 1, userInfo: [NSLocalizedDescriptionKey: "Failed to create transport"])
       }
        
        client = GRPCClient(transport: transport, interceptorPipeline: <#T##[ConditionalInterceptor<any ClientInterceptor>]#>)
        runClient()
    }
    
    func runClient() {
        Task {
            do {
                try await self.client.runConnections()
            } catch {
                print("Client closed with status: \(error)")
            }
        }
    }
    
    
    func stopClient() {
        self.client.beginGracefulShutdown()
    }
    
  
}
