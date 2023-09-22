#include "dispatch/core/RemoteDispatching/RemoteDispatcherClient.hh"


namespace claid
{
    RemoteDispatcherClient::RemoteDispatcherClient(const std::string& addressToConnectTo,
                    const std::string& userToken,
                    const std::string& deviceID,
                    SharedQueue<DataPackage>& incomingQueue, 
                    SharedQueue<DataPackage>& outgoingQueue) :  
                                                            userToken(userToken), deviceID(deviceID),
                                                            incomingQueue(incomingQueue), 
                                                            outgoingQueue(outgoingQueue)
    {
        // Set up the gRCP channel
        grpcChannel = grpc::CreateChannel(addressToConnectTo, grpc::InsecureChannelCredentials());
        stub = claidservice::ClaidRemoteService::NewStub(grpcChannel);
    }

    void RemoteDispatcherClient::shutdown() 
    {
        // Closing the outgoing queue will end the writer thread.
        outgoingQueue.close();
        if (this->writeThread) 
        {
            this->writeThread->join();
            this->writeThread = nullptr;
        }

        if (this->readThread) 
        {
            this->readThread->join();
            this->readThread = nullptr;
        }
    }

    void makeRemoteRuntimePing(ControlPackage& pkt, const std::string& userToken, const std::string& deviceID) 
    {
        pkt.set_ctrl_type(CtrlType::CTRL_REMOTE_PING);

        pkt.mutable_remote_client_info()->set_user_token(userToken);
        pkt.mutable_remote_client_info()->set_device_id(deviceID);
    }

    absl::Status RemoteDispatcherClient::registerAtServer() 
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

            std::cout << "checkpoint 15" << std::endl;

            claidservice::DataPackage pingReq;

            std::cout << "checkpoint 16" << std::endl;

            makeRemoteRuntimePing(*pingReq.mutable_control_val(), this->userToken, this->deviceID);

            std::cout << "checkpoint 17" << std::endl;

            if (!stream->Write(pingReq)) 
            {
                grpc::Status status = stream->Finish();
                return absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient failed to send ping package to server. Received error \"", status.error_message(), "\"\n";
                ));
            }

            std::cout << "Sent ping package to server !" << std::endl;

            // Wait for the valid response ping
            DataPackage pingResp;
            if (!stream->Read(&pingResp)) {
                auto status = stream->Finish();
                std::cout << "Did not receive a ping package from server. Error: \"" << status.error_message() << "\"" << std::endl;
                return absl::InvalidArgumentError(absl::StrCat(
                    "RemoteDispatcherClient failed to receive a ping package from the server. Received error \"", status.error_message(), "\"."
                ));
            }

            if (pingReq.control_val().ctrl_type() != CtrlType::CTRL_RUNTIME_PING) 
            {
                return false;
            }

            // Start the threads to service the input/output queues.
            std::cout << "Starting i/o threads !" << std::endl;
            writeThread = std::make_unique<std::thread>([this]() { processWriting(); });
            readThread = std::make_unique<std::thread>([this]() { processReading(); });

            std::cout << "Checkpoint 199" << std::endl;
            return true;
        }
    }

    void RemoteDispatcherClient::processReading() {
        DataPackage dp;
        std::cout << "Before reading" << std::endl;
        while(stream->Read(&dp)) {
            std::cout << "Read Packet " << std::endl;
            incomingQueue.push_back(std::make_shared<DataPackage>(dp));
        }
        std::cout << "After reading" << std::endl;
    }

    void RemoteDispatcherClient::processWriting() 
    {
        std::cout << "Before writing" << std::endl;

        while(true) 
        {
            auto pkt = outgoingQueue.pop_front();
            std::cout << "Writing packet to " << pkt << std::endl;
            if (!pkt) 
            {
                if (outgoingQueue.is_closed()) {
                    break;
                }
            } else {
                if (!stream->Write(*pkt)) {
                    // Server is down?
                    std::cout << "Error writing " << std::endl;
                    break;
                }
            }
        }
        std::cout << "After writing" << std::endl;
        stream->WritesDone();
    }
}