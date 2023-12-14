#include "dispatch/core/RemoteDispatching/RemoteClientHandler.hh"
#include "dispatch/core/Logger/Logger.hh"

using claidservice::CtrlType;


namespace claid
{
    RemoteClientHandler::RemoteClientHandler(SharedQueue<claidservice::DataPackage>& inQueue,
                                    SharedQueue<claidservice::DataPackage>& outQueue,
                                    const std::string& userToken, const std::string& deviceID) :
        incomingQueue(inQueue), outgoingQueue(outQueue),
        userToken(userToken), deviceID(deviceID)
    {
        this->active = false;
    }

    bool RemoteClientHandler::alreadyRunning() 
    {
        std::lock_guard<std::mutex> lock(this->writeThreadMutex);
        return writeThread.get() != nullptr;
    }

    grpc::Status RemoteClientHandler::startWriterThread(grpc::ServerReaderWriter<DataPackage, DataPackage>* stream) 
    {
        this->active = true;

        std::lock_guard<std::mutex> lock(this->writeThreadMutex);
        if (this->writeThread) 
        {
            return grpc::Status(grpc::INVALID_ARGUMENT, "Thread already running.");
        }

        this->writeThread = std::make_unique<std::thread>([this, stream]() {
            processWriting(stream);
        });

        return grpc::Status::OK;
    }

    void RemoteClientHandler::shutdownWriterThread() 
    {
        {  // only lock for checking the writer thread.
            std::lock_guard<std::mutex> lock(this->writeThreadMutex);
            if (!writeThread) 
            {
                return;
            }
        }

        // Terminate the writer thread and wait for it to finish.
        this->outgoingQueue.push_front(nullptr);
        this->writeThread->join();
        this->writeThread = nullptr;
    }

    void RemoteClientHandler::processWriting(grpc::ServerReaderWriter<DataPackage, DataPackage>* stream) 
    {
        while(this->active) 
        {
            auto pkt = this->outgoingQueue.pop_front();

            // If we got a null pointer we are done
            if (!pkt) 
            {
                Logger::logWarning("RemoteClientHandler::processWriting received null package");
                break;
            }

            if (!stream->Write(*pkt)) 
            {
                // Re-enqueue package.
                Logger::logWarning("RemoteClientHandler::processWriting failed to write, reenqueuing package");
                this->outgoingQueue.push_front(pkt);
                break;
            }
            else
            {
                Logger::logWarning("RemoteClientHandler::processWriting wrote package successfully");
            }
        }
        std::cout << "Done with writer thread of RemoteClientHandler!" << std::endl;
    }

    grpc::Status RemoteClientHandler::processReading(grpc::ServerReaderWriter<DataPackage, DataPackage>* stream) 
    {
        DataPackage inPkt;
        grpc::Status status;
        while(stream->Read(&inPkt) && this->active) 
        {
            processPacket(inPkt, status);
            if (!status.ok()) 
            {
                // TODO: remove once we have a cleanup function added.
                return status;
            }
        }
        return grpc::Status::OK;
    }

    void RemoteClientHandler::processPacket(DataPackage& pkt, grpc::Status& status) 
    {
        status = grpc::Status::OK;

        // Forward package to output queue, that's all we have to do.
        // The output queue is connected to the MasterRouter, which will forward
        // the package accordingly.
        if (!pkt.has_control_val())
        {
            this->incomingQueue.push_back(std::make_shared<DataPackage>(pkt));
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
                this->incomingQueue.push_back(response);
                break;
            }
            case CtrlType::CTRL_REMOTE_PING_PONG:
            {
                // Do nothing.
                break;
            }
            default: 
            {
                status = grpc::Status(grpc::INVALID_ARGUMENT, "Invalid ctrl type in RemoteClientHandler");
                break;
            }
        }
    }

    void RemoteClientHandler::shutdown()
    {
        std::cout << "Shutting down RemoteClientHandler\n";
        this->active = false;
        this->shutdownWriterThread();
    }

}

