#include "Utilities/Time.hpp"

#include "RunTime/RunTime.hpp"
#include "Binary/BinaryData.hpp"

namespace portaible
{
    // Class that reads data from a socket.
    // It is used by the RemoteModule class.
    // A RemoteModule is a module itself, which contains an instance of the SocketClientModule.
    // The SocketClientModule runs in a separate thread and notifies the RemoteModule
    // via a separate (local!) channel, when new data is available.



    class SocketClientReaderModule : public LocalModule
    {
        public: 
            SocketClientReaderModule();
            Channel<Request> subscribeToLocalRequestChannel(ChannelSubscriber<Request> subscriber, std::function<void (ChannelData<Request>)> function);


        private:
            // Only local channel, not visible from any other modules that it is
            // not explicitly handed over to.
            LocalTypedChannel<BinaryData> localDataChannel;
            
        


            
            const Duration getDurationToNextRequest();

            void initialize();


            void doScheduling()
            {
                std::shared_ptr<BinaryData> binaryData = std::make_shared<BinaryData>();
                
                if(!socketClient.read(*binaryData))
                {
                    this->onError();
                }

                // As binaryData is a shared_ptr, this does not result in a copy 
                // of the underlying binary data. 
                // As Channel::post is based on shared_ptrs internally (see Channel implementation and TaggedData implementation),
                // no data is copied whenever post() called with an shared_ptr instance of the Channel's data type.
                localDataChannel.post(binaryData);
            }

            void onError()
            {

            }


            
    
    };
}