#include "Network/SocketConnectionModule.hpp"
#include "Binary/BinarySerializer.hpp"

namespace portaible
{
namespace Network
{

    SocketConnectionModule::SocketConnectionModule(SocketClient& socketClient) : socketClient(socketClient), readerModule(&this->socketClient, this->registerToReceiveChannel(), this->registerToErrorChannel())
    {
        Logger::printfln("Socket client fd %d", socketClient.sock);
    }

    void SocketConnectionModule::setup()
    {
        Logger::printfln("SocketConnectionModule setup");
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
        Logger::printfln("Send message");
        BinaryData binaryData;
        BinarySerializer serializer;
        serializer.serialize(message);
        this->socketClient.write(serializer.binaryData);
    }


}
}