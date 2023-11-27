#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"
#include "dispatch/core/Logger/Logger.hh"

namespace claid
{

    RemoteDispatcherClient::~RemoteDispatcherClient()
    {
        if(this->running)
        {
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
        if(!this->running)
        {
            return;
        }
        std::cout << "Shutting down client 1\n";
        // Closing the outgoing queue will end the writer thread.
        // The writer thread will invoke stream->WritesDone() after the outgoingQueue was closed.
        // This will close the stream and also abort stream->Read() for the reader thread.
       
        if (this->writeThread) 
        {
            this->writeThread->join();
            this->writeThread = nullptr;
        }
        std::cout << "Shutting down client 2\n";

        if (this->readThread) 
        {
            this->readThread->join();
            this->readThread = nullptr;
        }
        std::cout << "Shutting down client 3\n";

        this->running = false;
    }

    void makeRemoteRuntimePing(ControlPackage& pkt, const std::string& host, 
                                const std::string& userToken, const std::string& deviceID) 
    {
        pkt.set_ctrl_type(CtrlType::CTRL_REMOTE_PING);

        pkt.mutable_remote_client_info()->set_host(host);
        pkt.mutable_remote_client_info()->set_user_token(userToken);    
        pkt.mutable_remote_client_info()->set_device_id(deviceID);
    }

    absl::Status RemoteDispatcherClient::registerAtServerAndStartStreaming() 
    {
        // Setup stream and send ping package, containg the user_token and device_id.
        // if (true)
        {
            // Set timeout for API
            // std::chrono::system_clock::time_point deadline =
            //     std::chrono::system_clock::now() + std::chrono::seconds(10);
            // context.set_deadline(deadline);

            streamContext = std::make_shared<grpc::ClientContext>();
            stream = stub->SendReceivePackages(streamContext.get());


            claidservice::DataPackage pingRequestPackage;
            makeRemoteRuntimePing(*pingRequestPackage.mutable_control_val(), this->host, this->userToken, this->deviceID);

            if (!stream->Write(pingRequestPackage)) 
            {
                grpc::Status status = stream->Finish();
                return absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient failed to send ping package to server. Received error \"", status.error_message(), "\"\n"
                ));
            }


            // Wait for the valid response ping
            DataPackage pingResp;
            if (!stream->Read(&pingResp)) {
                auto status = stream->Finish();
                return absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient failed to receive a ping package from the server. Received error \"", status.error_message(), "\"."
                ));
            }

            if (pingResp.control_val().ctrl_type() != CtrlType::CTRL_REMOTE_PING) 
            {
                return absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient received ControlPackage package from server during handshake, however the package has an unexpected ControlType.\n",
                    "Expected ControlType \"", CtrlType_Name(CtrlType::CTRL_REMOTE_PING), "\", but got \"", CtrlType_Name(pingResp.control_val().ctrl_type()), "\""
                ));
            }

            // Start the threads to service the input/output queues.
            writeThread = std::make_unique<std::thread>([this]() { processWriting(); });
            readThread = std::make_unique<std::thread>([this]() { processReading(); });

            this->running = true;

            return absl::OkStatus();
        }
    }

    void RemoteDispatcherClient::processReading() 
    {
        DataPackage dp;
        while(stream->Read(&dp)) 
        {
            this->clientTable.getFromRemoteClientQueue().push_back(std::make_shared<DataPackage>(dp));
        }
        std::cout << "RemoteDispatcherClient: Finished reading\n";
    }

    void RemoteDispatcherClient::processWriting() 
    {
        while(this->running) 
        {
            SharedQueue<DataPackage>& toRemoteClientQueue = this->clientTable.getToRemoteClientQueue();
            auto pkt = toRemoteClientQueue.pop_front();
            if (!pkt) 
            {
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
                    Logger::logWarning("Failed to write to remote server. RemoteDispatcherClient is shutting down queue.");
                    break;
                }
            }
        }
        stream->WritesDone();
        Logger::logInfo("RemoteDispatcherClient processWriting done");
    }
}