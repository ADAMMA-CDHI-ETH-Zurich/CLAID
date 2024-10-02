/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

#include "dispatch/core/RemoteDispatching/RemoteDispatcherServer.hh"

#include <grpcpp/server_builder.h>
#include <grpcpp/create_channel.h>

#include <grpc++/server_context.h>

#include <google/protobuf/text_format.h>



namespace claid 
{
    RemoteDispatcherServer::RemoteDispatcherServer(const std::string& addr, HostUserTable& hostUserTable)
        : addr(addr), remoteServiceImpl(hostUserTable)
    {

    }

    RemoteDispatcherServer::~RemoteDispatcherServer()
    {
        if(this->running)
        {
            this->shutdown();
        }
    }

    absl::Status RemoteDispatcherServer::start() 
    {
        if(this->running)
        {
            return absl::AlreadyExistsError("Failed to start RemoteDispatcherServer, server is already running (start was called twice).");
        }

        buildAndStartServer();
        if (!server) {
            return absl::InvalidArgumentError(
                absl::StrCat("Failed to start RemoteDispatcherServer with address \"", this->addr, "\"."));
        }

        this->running = true;

        return absl::OkStatus();
    }

    absl::Status RemoteDispatcherServer::start(const TLSServerKeyStore& serverKeyStore)
    {
        this->serverKeyStore = serverKeyStore;
        this->useTLS = true;
        return this->start();
    }

    void RemoteDispatcherServer::shutdown() 
    {
        if(!this->running)
        {
            return;
        }
        
        // server->Shutdown() will hang indefintely as long as there are still ongoing RPC calls by any client.
        // The clients call SendReceivePackages to stream data to/from the server. The SendReceivePackage RPC call
        // typically never returns, as long as the client is alive.
        // Hence, we have to forcefully end all client's RPC calls registered in the RemoteService.
        this->remoteServiceImpl.shutdown();
        // Now we can safely call server->Shutdown();

        // Without deadline, server->Shutdown() will wait indefinitely for all rpc calls to finish.
        // We would then need to let all clients know that they shall disconnect.
        // Alternatively, if we specify a deadline, all ongoing RPC calls are cancelled.
        const std::chrono::milliseconds waitDuration = std::chrono::milliseconds(500);
        const std::chrono::time_point<std::chrono::system_clock> deadline = std::chrono::system_clock::now() + waitDuration;

        server->Shutdown(deadline);

        this->running = false;
    }

    void RemoteDispatcherServer::buildAndStartServer()
    {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(addr, makeServerCredentials());
        builder.RegisterService(&remoteServiceImpl);
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS,
                                    10 * 60 * 1000 /*10 min*/);
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS,
                                    60 * 1000 /*60 sec*/);
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
        builder.AddChannelArgument(
            GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS,
            30 * 1000 /*30 sec*/);
        builder.SetMaxReceiveMessageSize(-1);  // -1 means unlimited

        // Set the maximum send message size (in bytes) for the server
        builder.SetMaxSendMessageSize(-1);  // -1 means unlimited

        server = builder.BuildAndStart();
    }

    std::shared_ptr<grpc::ServerCredentials> RemoteDispatcherServer::makeServerCredentials() const
    {
        if(this->useTLS)
        {
            grpc::SslServerCredentialsOptions::PemKeyCertPair key_cert_pair = {
                this->serverKeyStore.serverKey, 
                this->serverKeyStore.serverCert
            };
            grpc::SslServerCredentialsOptions ssl_opts;

            if(this->serverKeyStore.requiresMutualTLS())
            {
                // Mutual TLS (MTLS)
                ssl_opts.client_certificate_request = GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY;
                ssl_opts.pem_root_certs = this->serverKeyStore.clientCertificate;
            }
            ssl_opts.pem_key_cert_pairs.push_back(key_cert_pair);

            // Create server credentials
            auto server_creds = grpc::SslServerCredentials(ssl_opts);
            return server_creds;
        }
        else
        {
            return grpc::InsecureServerCredentials();
        }
    }

    bool RemoteDispatcherServer::isRunning() const
    {
        return this->running;
    }
}