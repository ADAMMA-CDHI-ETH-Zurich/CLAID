#include "Utilities/Time.hpp"

#include "RunTime/RunTime.hpp"
#include "Binary/BinaryData.hpp"
#include "Network/Message.hpp"

namespace portaible
{

    // It is used by the RemoteModule class.
    // A RemoteModule is a module itself, which contains an instance of the SocketClientModule.
    // The SocketClientModule runs in a separate thread and notifies the RemoteModule
    // via a separate (local!) channel, when new data is available.

    namespace Network
    {
        class NetworkModule : public SubModule
        {
            public: 
            //  SocketClientReaderModule();
        //     Channel<Request> subscribeToLocalRequestChannel(ChannelSubscriber<Request> subscriber, std::function<void (ChannelData<Request>)> function);
                
                
                virtual ~NetworkModule()
                {

                }

                virtual void sendMessage(Message message)
                {
                    this->callLater(&NetworkModule::asyncSendMessage, this, message);
                }

                virtual void subscribeToMessageChannel(ChannelSubscriber<Message> subscriber) = 0;
                

            private:

            


                
                virtual void asyncSendMessage(Message message) = 0;

                virtual void initialize()
                {
                    
                }

                void onError()
                {

                }


                
        
        };
    }

    
}