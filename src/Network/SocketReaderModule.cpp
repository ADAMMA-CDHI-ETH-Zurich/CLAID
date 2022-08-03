#include "Network/SocketReaderModule.hpp"
#include "Binary/BinaryDeserializer.hpp"
namespace portaible
{
namespace Network
{
       
    
    SocketReaderModule::SocketReaderModule() : socketClient(nullptr)
    {

    }
            
    SocketReaderModule::SocketReaderModule(SocketClient* socketClient) : 
                                            socketClient(socketClient)
    {

    }
    
    
    void SocketReaderModule::initialize()
    {
        // start might be called by another module / from another thread.
        // Thus, we make sure that we switch to our thread (of this module).
        this->active = true;
        this->stopped = false;
        Logger::printfln("call later");
        this->callLater(&SocketReaderModule::run, this);
    }

    void SocketReaderModule::run()
    {
        if(this->socketClient == nullptr)
        {
            PORTAIBLE_THROW(Exception, "Failed to start SocketReaderModule: SocketClient is nullptr " << __FILE__ << " line " << __LINE__);
        }

        while(this->active)
        {
            Logger::printfln("reading");
                    Logger::printfln("Socket client fd in reader %d", socketClient->sock);

            BinaryData binaryData;
            if(!this->socketClient->read(binaryData))
            {
                Logger::printfln("Read failed");
                postError<ErrorReadFromSocketFailed>();
                this->active = false;
                break;
            }
            Logger::printfln("read %d bytes", binaryData.getNumBytes());
            // BinaryData is a message -> deserialize to message?
            BinaryDeserializer deserializer;
            RemoteConnection::Message message;
            try
            {
                BinaryDataReader reader(&binaryData);
                deserializer.deserialize(message, reader);
                this->messageReceivedChannel.post(message);
            }
            catch (const Exception& e)
            {
                // TODO: IMPLEMENT PROPER ERROR HANDLING !!
                PORTAIBLE_THROW(Exception, "Caught exception in SocketReaderModule "  << e.what());
            }
        }
        this->stopped = true;
    }
              
    void SocketReaderModule::setMessageReceivedChannel(Channel<RemoteConnection::Message> messageReceivedChannel)
    {
        this->messageReceivedChannel = messageReceivedChannel;
    }
    
    
    void SocketReaderModule::setErrorChannel(Channel<RemoteConnection::Error> errorChannel)
    {
        this->errorChannel = errorChannel;
    }

    void SocketReaderModule::stop()
    {
        this->stopped = false;
        this->active = false;
    }

    bool SocketReaderModule::isStopped()
    {
        return this->stopped;
    }

    
   
}
}