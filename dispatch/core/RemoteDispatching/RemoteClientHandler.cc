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
        this->writeStream = stream;
        while(this->active) 
        {
            auto pkt = this->outgoingQueue.interruptable_pop_front();


            if (!pkt) {
                if(outgoingQueue.is_closed())
                {
                    break;
                }
                else
                {
                    // Spurious wakeup
                    continue;
                }
            }
            

            std::unique_lock<std::mutex>(this->pingMutex);
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
                Logger::logError("RemoteClientHandler: %s", status.error_message().c_str());
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
        if (pkt.control_val().ctrl_type() != CtrlType::CTRL_UNSPECIFIED)
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
                // Forward to middleware
                this->incomingQueue.push_back(std::make_shared<DataPackage>(pkt));
                break;
            }
        }
    }

    void RemoteClientHandler::shutdown()
    {
        std::cout << "Shutting down RemoteClientHandler\n";
        this->active = false;
        this->shutdownWriterThread();
        this->writeStream = nullptr;
    }

    bool RemoteClientHandler::sendPingToClient()
    {
        
        std::unique_lock<std::mutex>(this->pingMutex);
        if(this->writeStream == nullptr)
        {
            return false;
        }
        DataPackage pkt;
        pkt.mutable_control_val()->set_ctrl_type(CtrlType::CTRL_REMOTE_PING);
                
        return this->writeStream->Write(pkt);
    }

}

