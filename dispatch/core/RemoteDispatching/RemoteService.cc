#include "dispatch/core/RemoteDispatching/RemoteService.hh"

namespace claid
{

    RemoteServiceImpl::RemoteServiceImpl()
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


        // Add a RemoteClientHandler for this particular client and add it to the list of handlers.
        RemoteClientHandler* remoteClientHandler = addRemoteClientHandler(inPkt, status);
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
            return status;
        }
        std::cout << "Done starting writer thread\n";
        status = remoteClientHandler->processReading(stream);

        // HERE: processReading stopped, which means the client has lost connection.
        // TODO: IMPLEMENT SHUTDOWN
        std::cout << "Reading failed or ended: " << status.error_message() << "\n";
        return status;
    }


    RemoteClientHandler* RemoteServiceImpl::addRemoteClientHandler(DataPackage& pkt, grpc::Status& status) 
    {
        status = grpc::Status::OK;

        // Make sure we got a control package with a CTRL_REMOTE_PING message.
        auto ctrlType = pkt.control_val().ctrl_type();
        auto remoteClientInfo = pkt.control_val().remote_client_info();
        std::cout << "Got package:" << CtrlType_Name(ctrlType) << "   :    " << remoteClientInfo.user_token() << " " << remoteClientInfo.device_id() << std::endl;

        if (ctrlType != claidservice::CtrlType::CTRL_REMOTE_PING) 
        {
            status = grpc::Status(grpc::INVALID_ARGUMENT, 
                absl::StrCat("Runtime init failed. Expected control package with type CTRL_REMOTE_PING, but got: \"",
                claidservice::CtrlType_Name(ctrlType), "\""));
            return nullptr;
        }

        RemoteClientKey remoteClient = makeRemoteClientKey(remoteClientInfo);

        // check if the RemoteClientHandler exits
        std::lock_guard<std::mutex> lock(this->remoteClientHandlersMutex);
        auto it = this->remoteClientHandlers.find(remoteClient);
        if (it != this->remoteClientHandlers.end()) 
        {
            status = grpc::Status(grpc::ALREADY_EXISTS, absl::StrCat(
                "Failed to create RemoteClient handler for client with user token \"", remoteClientInfo.user_token(), "\"",
                "and device id \"", remoteClientInfo.device_id(), "\".\n",
                "A user with these identifiers already exists."
            ));
            return nullptr;
        }

        // Allocate a RemoteClientHandler
        SharedQueue<DataPackage>* input = new SharedQueue<DataPackage>();
        SharedQueue<DataPackage>* output = new SharedQueue<DataPackage>();
        auto remoteClientHandler = new RemoteClientHandler(*input, *output, remoteClientInfo.user_token(), remoteClientInfo.device_id());
        this->remoteClientHandlers[remoteClient] = std::unique_ptr<RemoteClientHandler>(remoteClientHandler);
        return remoteClientHandler;
    }
}