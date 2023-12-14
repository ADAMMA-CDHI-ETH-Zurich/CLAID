#include "dispatch/core/RemoteDispatching/RemoteService.hh"
#include "dispatch/core/Logger/Logger.hh"
namespace claid
{

    RemoteServiceImpl::RemoteServiceImpl(HostUserTable& hostUserTable) : hostUserTable(hostUserTable)
    {

    }

    grpc::Status RemoteServiceImpl::SendReceivePackages(grpc::ServerContext* context,
        grpc::ServerReaderWriter<claidservice::DataPackage, claidservice::DataPackage>* stream)
    {
        grpc::Status status;

        // Read the first packet. The first packed sent by the connected client
        // should be a control packet which includes the RemoteClientInfo of the client as payload.
        DataPackage inPkt;
        if (!stream->Read(&inPkt)) 
        {
            return grpc::Status(grpc::CANCELLED, "Unable to read input package from RemoteDispatcherClient!");
        }

        RemoteClientInfo remoteClientInfo;
        status = getRemoteClientInfoFromHandshakePackage(inPkt, remoteClientInfo);
        if(!status.ok())
        {
            return status;
        }

        std::cout << "Client \"" << makeRemoteClientIdentifier(remoteClientInfo) << " connected!\n";

        // Add a RemoteClientHandler for this particular client and add it to the list of handlers.
        RemoteClientHandler* remoteClientHandler = addRemoteClientHandler(remoteClientInfo, status);
        if (!status.ok()) 
        {
            return status;
        }

        std::cout << "RemoteClientHandler created !" << remoteClientHandler << " " << std::endl;

        // Return the package to the sender, to acknowledge that w
        DataPackage outPkt = inPkt;
        stream->Write(outPkt);

        std::cout << "Starting writer thread !" << std::endl;

        status = remoteClientHandler->startWriterThread(stream);
        if(!status.ok())
        {
            Logger::logWarning("RemoteClientHandler failed to accept client: %s", status.error_message().c_str());
            return status;
        }
        std::cout << "Done starting writer thread\n";
        status = remoteClientHandler->processReading(stream);

        // HERE: processReading stopped, which means the client has lost connection.
        // TODO: IMPLEMENT SHUTDOWN
        std::cout << "Reading failed or ended: " << status.error_message() << "\n";
        std::cout << "Client \"" << makeRemoteClientIdentifier(remoteClientInfo) << " disconnected or died!\n";
        std::cout << "Unregistering and forgetting client!\n";

        this->stopAndRemoveRemoteClientHandler(remoteClientInfo);
        std::cout << "And we are done! Bye bye client, see you next time\n";

        return status;
    }

    grpc::Status RemoteServiceImpl::getRemoteClientInfoFromHandshakePackage(const DataPackage& package, RemoteClientInfo& remoteClientInfo)
    {
        // Make sure we got a control package with a CTRL_REMOTE_PING message.
        auto ctrlType = package.control_val().ctrl_type();

        if (ctrlType != claidservice::CtrlType::CTRL_REMOTE_PING) 
        {
            return grpc::Status(grpc::INVALID_ARGUMENT, 
                absl::StrCat("Runtime init failed. Expected control package with type CTRL_REMOTE_PING, but got: \"",
                claidservice::CtrlType_Name(ctrlType), "\""));
        }

        remoteClientInfo = package.control_val().remote_client_info();
        std::cout << "Got package:" << CtrlType_Name(ctrlType) << "   :    " << remoteClientInfo.user_token() << " " << remoteClientInfo.device_id() << std::endl;

        return grpc::Status::OK;   
    }


    RemoteClientHandler* RemoteServiceImpl::addRemoteClientHandler(const RemoteClientInfo& remoteClientInfo, grpc::Status& status) 
    {
        status = grpc::Status::OK;

        RemoteClientKey remoteClient = makeRemoteClientKey(remoteClientInfo);

        // check if the RemoteClientHandler exits
        std::lock_guard<std::mutex> lock(this->remoteClientHandlersMutex);
        auto it = this->remoteClientHandlers.find(remoteClient);
        std::cout << "RemoteClientHandlers size " << this->remoteClientHandlers.size() << "\n";
        if (it != this->remoteClientHandlers.end()) 
        {
            status = grpc::Status(grpc::ALREADY_EXISTS, absl::StrCat(
                "Failed to create RemoteClient handler for client with user token \"", remoteClientInfo.user_token(), "\"",
                "and device id \"", remoteClientInfo.device_id(), "\".\n",
                "A user with these identifiers already exists."
            ));
            Logger::logWarning("%s", status.error_message().c_str());
            return nullptr;
        }
        // Allocate a RemoteClientHandler
        absl::Status abslStatus = 
            this->hostUserTable.addRemoteClient(
                remoteClientInfo.host(), remoteClientInfo.user_token(), remoteClientInfo.device_id());


        if(!abslStatus.ok())
        {
            Logger::logInfo("RemoteService fail %s", abslStatus.ToString().c_str());
            status = grpc::Status(grpc::CANCELLED, abslStatus.ToString());
            return nullptr;
        }


        std::shared_ptr<SharedQueue<DataPackage>> fromRemoteClientQueue = this->hostUserTable.inputQueue();
        std::shared_ptr<SharedQueue<DataPackage>> toRemoteClientQueue;
        
        abslStatus = 
            this->hostUserTable.lookupOutputQueueForHostUser(
                    remoteClientInfo.host(), remoteClientInfo.user_token(), toRemoteClientQueue);

        if(!abslStatus.ok())
        {
            status = grpc::Status(grpc::CANCELLED, abslStatus.ToString());
            Logger::logInfo("RemoteService fail %s", abslStatus.ToString().c_str());
            return nullptr;
        }

        auto remoteClientHandler = 
            new RemoteClientHandler(
                    *fromRemoteClientQueue, *toRemoteClientQueue, 
                    remoteClientInfo.user_token(), remoteClientInfo.device_id());

        this->remoteClientHandlers[remoteClient] = 
            std::unique_ptr<RemoteClientHandler>(remoteClientHandler);
        return remoteClientHandler;
    }

    void RemoteServiceImpl::stopAndRemoveRemoteClientHandler(const RemoteClientInfo& remoteClientInfo)
    {
        RemoteClientKey remoteClient = makeRemoteClientKey(remoteClientInfo);

        std::lock_guard<std::mutex> lock(this->remoteClientHandlersMutex);
        auto it = this->remoteClientHandlers.find(remoteClient);

        if(it == this->remoteClientHandlers.end())
        {
            std::cout << "Unable to remove RemoteClient \"" << makeRemoteClientIdentifier(remoteClientInfo) << "\".\n"
            << "A client with these identifiers does not exist.\n"; 
            return;
        }

        it->second->shutdown();
        this->remoteClientHandlers.erase(it); // Removes the unique_ptr from the map, which will cause the managed object to be deleted.
        
        absl::Status status = this->hostUserTable.removeRemoteClient(remoteClientInfo.host(), remoteClientInfo.user_token(), remoteClientInfo.device_id());

        if(!status.ok())
        {
            Logger::logWarning("Failed to remove host %s:%s:%s from HostUserTable: %s", 
            remoteClientInfo.host().c_str(), remoteClientInfo.user_token().c_str(), remoteClientInfo.device_id().c_str(),  status.ToString().c_str());
        }
    }

    void RemoteServiceImpl::shutdown()
    {
        for(auto& clientHandler : this->remoteClientHandlers)
        {
            std::cout << "Shutting down client handler for client " << clientHandler.first.first << " " << clientHandler.first.second << "\n";
            clientHandler.second->shutdown();
        }
    }
}