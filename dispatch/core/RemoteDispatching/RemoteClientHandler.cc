#include "dispatch/core/RemoteDispatching/RemoteClientHandler.hh"


namespace claid
{
    RemoteClientHandler::RemoteClientHandler(SharedQueue<claidservice::DataPackage>& inQueue,
                                    SharedQueue<claidservice::DataPackage>& outQueue,
                                    const std::string& userToken, const std::string& deviceID) :
        incomingQueue(inQueue), outgoingQueue(outQueue),
        userToken(userToken), deviceID(deviceID)
    {

    }

    bool RemoteClientHandler::alreadyRunning() 
    {
        std::lock_guard<std::mutex> lock(this->writeThreadMutex);
        return writeThread.get() != nullptr;
    }

    grpc::Status RemoteClientHandler::startWriterThread(grpc::ServerReaderWriter<DataPackage, DataPackage>* stream) 
    {
        std::cout << "RemoteClientHandler startWriterThread ckpt 1\n";
        std::lock_guard<std::mutex> lock(this->writeThreadMutex);
        if (this->writeThread) 
        {
            return grpc::Status(grpc::INVALID_ARGUMENT, "Thread already running.");
        }
        std::cout << "RemoteClientHandler startWriterThread ckpt 2\n";
        this->writeThread = std::make_unique<std::thread>([this, stream]() {
            processWriting(stream);
        });
        std::cout << "RemoteClientHandler startWriterThread ckpt 3\n";
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
        while(true) 
        {
            auto pkt = outgoingQueue.pop_front();

            // If we got a null pointer we are done
            if (!pkt) 
            {
                break;
            }

            if (!stream->Write(*pkt)) 
            {
                // Re-enqueue package.
                outgoingQueue.push_front(pkt);
                break;
            }
        }
        std::cout << "Done with writer thread of RemoteClientHandler!" << std::endl;
    }

    grpc::Status RemoteClientHandler::processReading(grpc::ServerReaderWriter<DataPackage, DataPackage>* stream) 
    {
        std::cout << "RemoteClientHandler reading 1 \n";
        DataPackage inPkt;
        grpc::Status status;
        while(stream->Read(&inPkt)) 
        {
            std::cout << "RemoteClientHandler reading 2 \n";
            processPacket(inPkt, status);
            if (!status.ok()) 
            {
                std::cout << "RemoteClientHandler reading 3 " << status.error_message() << "\n";
                // TODO: remove once we have a cleanup function added.
                return status;
            }
        }
        std::cout << "RemoteClientHandler reading end \n";
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
            this->outgoingQueue.push_back(std::make_shared<DataPackage>(pkt));
            return;
        }
        // Process the control values
        auto ctrlType = pkt.control_val().ctrl_type();
        switch(ctrlType) 
        {
            default: 
            {
                status = grpc::Status(grpc::INVALID_ARGUMENT, "Invalid ctrl type in RemoteClientHandler");
                break;
            }
        }
    }

}

