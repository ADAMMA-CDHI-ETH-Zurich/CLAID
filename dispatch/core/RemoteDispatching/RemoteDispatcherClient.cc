#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"
#include "dispatch/core/Logger/Logger.hh"

namespace claid
{

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
        grpcChannel = grpc::CreateChannel(addressToConnectTo, grpc::InsecureChannelCredentials());
        stub = claidservice::ClaidRemoteService::NewStub(grpcChannel);
    }

    void RemoteDispatcherClient::shutdown() 
    {
        if(!this->connectionMonitorRunning)
        {
            return;
        }
        std::cout << "Shutting down client 1\n";
        // Closing the outgoing queue will end the writer thread.
        // The writer thread will invoke stream->WritesDone() after the outgoingQueue was closed.
        // This will close the stream and also abort stream->Read() for the reader thread.
       
        this->connectionMonitorRunning = false;
        this->connected = false;
        this->stream->Finish();

        if (this->writeThread) 
        {
            this->writeThread->join();
            this->writeThread = nullptr;
        }
        std::cout << "Shutting down client 2\n";

        if (this->watcherAndReaderThreader) 
        {
            this->watcherAndReaderThreader->join();
            this->watcherAndReaderThreader = nullptr;
        }
        std::cout << "Shutting down client 3\n";

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

            auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(2);
            bool connected = grpcChannel->WaitForConnected(deadline);


            grpc_connectivity_state state = grpcChannel->GetState(/* try_to_connect */ true);

            if (!(connected && state == GRPC_CHANNEL_READY))
            {
                Logger::logInfo("UNAVAILABLE");
                this->lastStatus = absl::UnavailableError("RemoteDispatcherClient failed to connect to remote server. Server unreachable.");
                continue;
            }

            stream = stub->SendReceivePackages(streamContext.get());

            if(!stream)
            {
                this->lastStatus = absl::UnavailableError("RemoteDispatcherClient failed to connect to remote server. Stream is null.");
            }
            
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

            // Start the thread to service the input/output queues.
            writeThread = std::make_unique<std::thread>([this]() { processWriting(); });
            
            Logger::logInfo("RemoteDispatcherClient setup successful");
            
            // Blocks as long as we are connected
            processReading();

            Logger::logInfo("RemoteDispatcherClient lost connection, stopping reader and writer.");
            this->connected = false;
            this->stream->Finish();
            
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
        DataPackage dp;
        while(stream->Read(&dp)) 
        {
            this->clientTable.getFromRemoteClientQueue().push_back(std::make_shared<DataPackage>(dp));
        }
        Logger::logInfo("RemoteDispatcherClient stream closed");

    }

    void RemoteDispatcherClient::processWriting() 
    {
        Logger::logInfo("RemoteDispatcherClient: processWriting()");

        while(this->connected) 
        {
            SharedQueue<DataPackage>& toRemoteClientQueue = this->clientTable.getToRemoteClientQueue();
            auto pkt = toRemoteClientQueue.pop_front();
            if (!pkt) 
            {
                Logger::logInfo("RemoteDispatcherClient: pkt is null");
                if (toRemoteClientQueue.is_closed()) 
                {
                    break;
                }
            } 
            else 
            {
                if (!stream->Write(*pkt)) 
                {
                    // Server is down?
                    Logger::logWarning("Failed to write to remote server. RemoteDispatcherClient is lost connection.");
                    break;
                }
            }
        }
        stream->WritesDone();
        Logger::logInfo("RemoteDispatcherClient processWriting done");
    }

    bool RemoteDispatcherClient::isConnected() const
    {
        return this->connected;
    }
    
    absl::Status RemoteDispatcherClient::getLastStatus() const
    {
        return this->lastStatus;   
    }
}