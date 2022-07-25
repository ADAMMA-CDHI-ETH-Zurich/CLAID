#include "Network/SocketConnectionModule.hpp"
#include "Binary/BinarySerializer.hpp"

namespace portaible
{
namespace Network
{


    void SocketConnectionModule::start(SocketClient* socketClient)
    {
        if(this->started)
        {
            PORTAIBLE_THROW(Exception, "Error! Start was called twice on SocketConnectionModule.");
        }
        this->started = true;

        this->socketClient = socketClient;
        this->readerModule = new SocketReaderModule(socketClient, this->registerToSendChannel(), this->registerToErrorChannel());
        this->readerModule->startModule();
        
    }

    void SocketConnectionModule::sendMessage(RemoteConnection::Message message)
    {
        BinaryData binaryData;
        BinarySerializer serializer;
        serializer.serialize(message);
        this->socketClient->write(serializer.binaryData);
    }


}
}