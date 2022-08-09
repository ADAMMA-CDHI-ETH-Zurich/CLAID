#include "Network/SocketConnectionModule.hpp"
#include "Binary/BinarySerializer.hpp"

namespace portaible
{
namespace Network
{

    SocketConnectionModule::SocketConnectionModule(SocketClient socketClient) : socketClient(socketClient), readerModule(&this->socketClient)
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

        this->readerModule.setMessageReceivedChannel(this->registerToReceiveChannel());
        this->readerModule.setErrorChannel(this->registerToErrorChannel());
 
     
        
    }

    void SocketConnectionModule::start()
    {
        // Why start not in setup ? 
        // Because, maybe an external module wants to subscribe to our error channel before calling start.
        // It is only allowed to subscribe/publish channels during initialization/setup. Therefore,
        // an external Module (e.g. the RemoteConnectedEntity), that uses/start the ConnectionModule, 
        // can only subscribe to the error channel AFTER calling startModule() (which calls setup()).
        // Thus, after calling startModule(), the external module can subscribe to the error channel and then
        // call start in order to start the ConnectionModule().
        this->readerModule.startModule();
        this->readerModule.waitForInitialization();
    }

    void SocketConnectionModule::sendMessage(RemoteConnection::Message message)
    {
        Logger::printfln("Send message");
        BinaryData binaryData;
        BinarySerializer serializer;
        serializer.serialize(message, &binaryData);
        this->socketClient.write(binaryData);
    }


}
}