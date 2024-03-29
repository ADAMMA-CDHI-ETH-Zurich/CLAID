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

#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"
#include "dispatch/core/Logger/Logger.hh"

namespace claid
{
    Duration RemoteDispatcherClient::MAX_TIME_WITHOUT_PACKAGE_BEFORE_TESTING_TIMEOUT = Duration::minutes(5);

    RemoteDispatcherClient::~RemoteDispatcherClient()
    {
        if(this->connectionMonitorRunning)
        {
            Logger::logInfo("RemoteDispatcherClient: destructor");
            shutdown();
        }
    }

    RemoteDispatcherClient::RemoteDispatcherClient(const std::string& addressToConnectTo,
                    const std::string& host,
                    const std::string& userToken,
                    const std::string& deviceID,
                    ClientTable& clientTable) :     host(host), userToken(userToken), 
                                                    deviceID(deviceID), clientTable(clientTable)
    {
        // Set up the gRCP channel
        grpc::ChannelArguments args;
        // Sample way of setting keepalive arguments on the client channel. Here we
        // are configuring a keepalive time period of 20 seconds, with a timeout of 10
        // seconds. Additionally, pings will be sent even if there are no calls in
        // flight on an active connection.
        args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 10 * 60 * 1000 /* 10 minutes sec*/);
        args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 20 * 1000 /*10 sec*/);
        args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
        args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 1024 * 1024 * 1024);  // 1 GB
    
        // Set the maximum send message size (in bytes)
        args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, 1024 * 1024 * 1024);  // 1 GB
    


        grpcChannel = grpc::CreateCustomChannel(addressToConnectTo, grpc::InsecureChannelCredentials(), args);
        stub = claidservice::ClaidRemoteService::NewStub(grpcChannel);
    }

    void RemoteDispatcherClient::shutdown() 
    {
        if(!this->connectionMonitorRunning)
        {
            return;
        }
        // Closing the outgoing queue will end the writer thread.
        // The writer thread will invoke stream->WritesDone() after the outgoingQueue was closed.
        // This will close the stream and also abort stream->Read() for the reader thread.
       
        this->connectionMonitorRunning = false;
        this->connected = false;
        
       // this->stream->Finish();


        if (this->writeThread) 
        {
            this->writeThread->join();
            this->writeThread = nullptr;
        }

        if (this->watcherAndReaderThreader) 
        {
            this->watcherAndReaderThreader->join();
            this->watcherAndReaderThreader = nullptr;
        }

    }

    void makeRemoteRuntimePing(ControlPackage& pkt, const std::string& host, 
                                const std::string& userToken, const std::string& deviceID) 
    {
        pkt.set_ctrl_type(CtrlType::CTRL_REMOTE_PING);

        pkt.mutable_remote_client_info()->set_host(host);
        pkt.mutable_remote_client_info()->set_user_token(userToken);    
        pkt.mutable_remote_client_info()->set_device_id(deviceID);
    }


    void RemoteDispatcherClient::connectAndMonitorConnection() 
    {
        while(this->connectionMonitorRunning)
        {
            Logger::logInfo("RemoteDispatcherClient is trying to establish a connection.");
            streamContext = std::make_shared<grpc::ClientContext>();

            const int RECONNECT_TIMEOUT_SECONDS = 10;
            auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(RECONNECT_TIMEOUT_SECONDS);
            // Note: If the server is unreachable, this will wait 10 seconds.
            // HOWEVER: If the server is reachable, but it denies our request (does not accept connection), this will not wait.
            // ONLY waits if server is NOT reachable. Does NOT wait if we connect successfully OR server rejects us :)
            bool connected = grpcChannel->WaitForConnected(deadline);


            grpc_connectivity_state state = grpcChannel->GetState(/* try_to_connect */ true);

            if (!(connected && state == GRPC_CHANNEL_READY))
            {
                Logger::logInfo("SERVER UNAVAILABLE");
                this->lastStatus = absl::UnavailableError("RemoteDispatcherClient failed to connect to remote server. Server unreachable.");
                continue;
            }

            Logger::logInfo("RemoteDispatcherClient 1.");
            stream = stub->SendReceivePackages(streamContext.get());

            Logger::logInfo("RemoteDispatcherClient 2.");
            if(!stream)
            {
                this->lastStatus = absl::UnavailableError("RemoteDispatcherClient failed to connect to remote server. Stream is null.");
                continue;
            }
            Logger::logInfo("RemoteDispatcherClient 3.");

            claidservice::DataPackage pingRequestPackage;
            makeRemoteRuntimePing(*pingRequestPackage.mutable_control_val(), this->host, this->userToken, this->deviceID);

            Logger::logInfo("Sending ping package");



            if (!stream->Write(pingRequestPackage)) 
            {
                grpc::Status status = stream->Finish();
                this->lastStatus = absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient failed to send ping package to server. Received error \"", status.error_message(), "\"\n"
                ));
            }                
        

            


            Logger::logInfo("Waiting for ping response");

            // Wait for the valid response ping
            DataPackage pingResp;
            if (!stream->Read(&pingResp)) {
                auto status = stream->Finish();
                this->lastStatus = absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient failed to receive a ping package from the server. Received error \"", status.error_message(), "\"."
                ));
            }

            if (pingResp.control_val().ctrl_type() != CtrlType::CTRL_REMOTE_PING) 
            {
                this->lastStatus = absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient received ControlPackage package from server during handshake, however the package has an unexpected ControlType.\n",
                    "Expected ControlType \"", CtrlType_Name(CtrlType::CTRL_REMOTE_PING), "\", but got \"", CtrlType_Name(pingResp.control_val().ctrl_type()), "\""
                ));
            }
            this->connected = true;
            this->lastStatus = absl::OkStatus();

            this->lastTimePackageWasSent = Time::now();

            // Start the thread to service the input/output queues.
            writeThread = std::make_unique<std::thread>([this]() { processWriting(); });
            
            Logger::logInfo("RemoteDispatcherClient setup successful");
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

            this->onConnectedToServer();
            // Blocks as long as we are connected
            processReading();

            this->onDisconnectedFromServer();

            Logger::logInfo("RemoteDispatcherClient lost connection, stopping reader and writer.");
            this->connected = false;
            this->stream->WritesDone();
            this->clientTable.getToRemoteClientQueue().interruptOnce();

            // TODO: Check if we have to call stream->Finish().
            // By this point, the stream should already be cancelled/ended, becase processReading keeps blocking
            // as long as the stream is open and read is successful. If we reach this part of the code, that means
            // stream->Read() failed. Also, if the Server rejects our connection request (e.g., because there is already
            // a client connected with the same user id), the application would crash if we call stream->Finish() due to 
            // "API misuse of type GRPC_CALL_ERROR_TOO_MANY_OPERATIONS observed".
           // this->stream->Finish();
            
            Logger::logInfo("RemoteDispatcherClient waiting for writerThread to join.");
            writeThread->join();
            writeThread = nullptr;
            Logger::logInfo("RemoteDispatcherClient is reset.");
        }
        
    
    }

    absl::Status RemoteDispatcherClient::start()
    {
        if(this->connectionMonitorRunning)
        {
            return absl::AlreadyExistsError("RemoteDispatcherClient was started twice");
        }
        this->connectionMonitorRunning = true;
        watcherAndReaderThreader = std::make_unique<std::thread>([this]() { connectAndMonitorConnection(); });
        return absl::OkStatus();
    }

    void RemoteDispatcherClient::processReading() 
    {
        Logger::logInfo("ProcessReading");
        DataPackage dp;
        while(stream->Read(&dp)) 
        {
            processPacket(dp);
        }
        Logger::logInfo("RemoteDispatcherClient stream closed");

    }

    void RemoteDispatcherClient::processPacket(DataPackage& pkt)
    {

        if (!pkt.has_control_val())
        {
            this->clientTable.getFromRemoteClientQueue().push_back(std::make_shared<DataPackage>(pkt));
            return;
        }
        // Process the control values
        auto ctrlType = pkt.control_val().ctrl_type();
        switch(ctrlType) 
        {
            case CtrlType::CTRL_REMOTE_PING:
            {
                // Received ping, TODO add response.
                std::shared_ptr<DataPackage> response = std::make_shared<DataPackage>();
                response->set_target_host(pkt.source_host());
                auto ctrlMsg = response->mutable_control_val();
                ctrlMsg->set_ctrl_type(CtrlType::CTRL_REMOTE_PING_PONG);
                this->clientTable.getFromRemoteClientQueue().push_back(response);
                break;
            }
            case CtrlType::CTRL_REMOTE_PING_PONG:
            {
                // Do nothing.
                break;
            }
            default: 
            {
                // Forward to middleware
                this->clientTable.getFromRemoteClientQueue().push_back(std::make_shared<DataPackage>(pkt));
                break;
            }
        }


    }


        

    void RemoteDispatcherClient::processWriting() 
    {
        Logger::logInfo("RemoteDispatcherClient: processWriting()");

        while(this->connected) 
        {
            SharedQueue<DataPackage>& toRemoteClientQueue = this->clientTable.getToRemoteClientQueue();
            auto pkt = toRemoteClientQueue.interruptable_pop_front();
            if (!pkt) 
            {
                // It's alright, null pkt can happen due to spurious wakeups or when toRemoteClientQueue.interruptOnce() is called.
                // interruptable_pop_front() waits but can be woken up due to spurious wakeups, in contrast to pop_front() which will always 
                // ever return if data is really available, or the channel is closed.
                if (toRemoteClientQueue.is_closed()) 
                {
                    Logger::logInfo("RemoteDispatcherClient: pkt is null and queue is closed, shutting down.");
                    break;
                }
            } 
            else 
            {
                // If the last time we send a package is > then CONNECTION_TIMEOUT_INTERVAL,
                // we first send a ping package to the Server to test whether the connection is alive.
                // If we or the server lost connection to the internet, then the connection might have faded silently,
                // and our gRPC stream has not noticed it yet, because we have not sent a package for a while.
                // Hence, before we send the actual package, we first send a ping package to test whether the stream is alive.
                // We do not care about the response to that ping, but only whether stream->Write() still returns successfully.
                // If it does not, the connection is dead and we reenqueue the package to be sent later, when we have reconnected.

                Time currentTime = Time::now();
                Duration timeSinceLastPackage = currentTime.subtract(this->lastTimePackageWasSent);
                if(timeSinceLastPackage > MAX_TIME_WITHOUT_PACKAGE_BEFORE_TESTING_TIMEOUT)
                {
                    claidservice::DataPackage pingRequestPackage;
                    makeRemoteRuntimePing(*pingRequestPackage.mutable_control_val(), this->host, this->userToken, this->deviceID);
                    if(!stream->Write(pingRequestPackage))
                    {
                        Logger::logWarning("RemoteDispatcherClient tried to send a message to the server after not having sent a message for %d minutes.\n",
                                           "Since a package has not been sent for a while, a ping package was sent to the server to test whether the connection is still alive.\n",
                                           "Such a ping package will be sent before an actual package, if the last actual package was sent more than %d minutes ago.\n",
                                           "While sending that ping package, it was noticed that the connection is actually dead. Hence, we are closing the connection on\n",
                                           "The side of the RemoteDispatcherClient as well and try to reconnect soon. The actual package will be reenqueued to be sent once the connection is reestablished.");
                        toRemoteClientQueue.push_front(pkt);
                        break;
                    }
                    
                }
                

                if (!stream->Write(*pkt)) 
                {
                    // Server is down?
                    Logger::logWarning("Failed to write to remote server. RemoteDispatcherClient is lost connection.");
                    break;
                }
                this->lastTimePackageWasSent = Time::now();
            }
        }
        Logger::logInfo("RemoteDispatcherClient processWriting done");
    }

    bool RemoteDispatcherClient::isConnected() const
    {
        return this->connected;
    }

    void RemoteDispatcherClient::onConnectedToServer()
    {
        SharedQueue<DataPackage>& outQueue = this->clientTable.getFromRemoteClientQueue();

        // Send a package to the middleware to notify that we are connected to the serversuccessfully.
        // The middleware might forward this package to all local modules to notify them about the successfull connection.

        std::shared_ptr<DataPackage> onConnectPackage = std::make_shared<DataPackage>();

        onConnectPackage->mutable_control_val()->set_ctrl_type(CtrlType::CTRL_CONNECTED_TO_REMOTE_SERVER);
        onConnectPackage->set_source_host(this->host);
        onConnectPackage->set_target_host(this->host);

        outQueue.push_back(onConnectPackage);
    }

    void RemoteDispatcherClient::onDisconnectedFromServer()
    {
        SharedQueue<DataPackage>& outQueue = this->clientTable.getFromRemoteClientQueue();

        // Send a package to the middleware to notify that we are disconnected from the server.
        // The middleware might forward this package to all local modules to notify them about the successfull connection.

        std::shared_ptr<DataPackage> onDisconnectPackage = std::make_shared<DataPackage>();

        onDisconnectPackage->mutable_control_val()->set_ctrl_type(CtrlType::CTRL_DISCONNECTED_FROM_REMOTE_SERVER);
        onDisconnectPackage->set_source_host(this->host);
        onDisconnectPackage->set_target_host(this->host);
        outQueue.push_back(onDisconnectPackage);
    }
    
    absl::Status RemoteDispatcherClient::getLastStatus() const
    {
        return this->lastStatus;   
    }
}