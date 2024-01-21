#include "Network/SocketConnectionModule.hpp"
#include "Binary/BinarySerializer.hpp"
#include "RemoteConnection/Message/MessageHeader/MessageHeaderChannelData.hpp"
#include "Logger/Logger.hpp"
namespace claid
{
namespace Network
{

    SocketConnectionModule::SocketConnectionModule(SocketClient socketClient) : socketClient(socketClient), readerModule(&this->socketClient)
    {
        Logger::logInfo("Socket client fd %d", socketClient.sock);
    }

    void SocketConnectionModule::setup()
    {
        Logger::logInfo("SocketConnectionModule setup");
        if(this->started)
        {
            CLAID_THROW(Exception, "Error! Start was called twice on SocketConnectionModule.");
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

    void SocketConnectionModule::stop()
    {
        Logger::logInfo("SocketConnectionModule::Closing socket");
        this->socketClient.close();

        Logger::logInfo("SocketConnectionModule::Stopping reader module");
        this->readerModule.stop();

        while(!this->readerModule.isStopped())
        {
            // Waiting
            Logger::logInfo("waiting for stopping reader module");
        }

        


        // waits until the module (thread) has been stopped
        // Unpublishes messageReceive and errorChannel
        this->readerModule.stopModule();
    }

    void SocketConnectionModule::sendMessage(RemoteConnection::Message message)
    {
        Logger::logInfo("SocketConnectionModule::sendMessage");
      
        // if(message.header->is<RemoteConnection::MessageHeaderChannelData>())
        // {
        //     Logger::logInfo("Skipping message");
        //     return;
        // }
        BinaryData binaryData;
        BinarySerializer serializer;
        serializer.serialize(message, &binaryData);
        if(!this->socketClient.write(binaryData))
        {
            this->postError<ErrorReadFromSocketFailed>();
        }
    }


}
}

// class GenericReflector
// {
//     AnyReflector reflector;

//     template<typename T>
//     void member(T& t)
//     {
//         reflector.member(t);
//     }
// };