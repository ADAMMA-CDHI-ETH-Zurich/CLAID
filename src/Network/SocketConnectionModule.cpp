#include "Network/SocketConnectionModule.hpp"
#include "Binary/BinarySerializer.hpp"

namespace portaible
{
namespace Network
{

    SocketConnectionModule::SocketConnectionModule(SocketClient& socketClient) : socketClient(socketClient), readerModule(&this->socketClient, this->registerToSendChannel(), this->registerToErrorChannel())
    {

    }

    void SocketConnectionModule::setup()
    {
        if(this->started)
        {
            PORTAIBLE_THROW(Exception, "Error! Start was called twice on SocketConnectionModule.");
        }
        this->started = true;

        this->readerModule.startModule();
        this->readerModule.waitForInitialization();
        
    }

    void SocketConnectionModule::sendMessage(RemoteConnection::Message message)
    {
        BinaryData binaryData;
        BinarySerializer serializer;
        serializer.serialize(message);
        this->socketClient.write(serializer.binaryData);
    }


}
}